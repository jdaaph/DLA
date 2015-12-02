/* HPCU Bi-Weekly Challenge: Diffusion, Limited Aggregation in MPI
 * Solution
 * Aaron Weeden, Shodor, 2015
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <mpi.h>

#define FREE 0
#define STUCK 1

/* Important for random number generation that these be between 0 and 3,
 * inclusive */
#define UP 0
#define LEFT 1
#define RIGHT 2
#define DOWN 3

/* Important that this be a different value than the 4 above */
#define STAY -1

#define DEFAULT_NUM_FREE 200
#define DEFAULT_NUM_STUCK 1
#define NUM_PARTICLES 201
#define DEFAULT_STICKINESS 100
#define DEFAULT_INTERVAL_USECS 10000

const int ENVIRONMENT_WIDTH = 100;
const int ENVIRONMENT_HEIGHT = 100;
const char FREE_COLOR[] = "#0000FF";
const char STUCK_COLOR[] = "#FF0000";
const char BACKGROUND_COLOR[] = "#FFFFFF";
const int PARTICLE_HEIGHT = 5;
const int PARTICLE_WIDTH = 5;

int Stickiness = DEFAULT_STICKINESS;
int IntervalUsecs = DEFAULT_INTERVAL_USECS;
int ParticleXs[NUM_PARTICLES];
int ParticleYs[NUM_PARTICLES];
int ParticleStates[NUM_PARTICLES];
int RandomSeed;
bool RunForever = true;
int CurTime = 0;
int NumTimeSteps;
int MPIRank;
int MPISize;
/* So all processes know how big of a message to receive from other processes,
 * keep track of the following data, 1 array element per MPI process (rank 0 is 
 * not included in the arrays, since it does not send messages. However, the
 * arrays are still indexed by MPI rank, starting at 0). */
int *ParticleCounts;
int *ParticleDispls;

Display *X11_Display;
Window X11_Window;
int X11_Screen;
GC X11_GC;
XColor X11_FreeColor;
XColor X11_StuckColor;
Colormap X11_ColorMap;

/* Initialize the X Windows Environment
 * This all comes from
 *   http://en.wikibooks.org/wiki/X_Window_Programming/XLib
 *   http://tronche.com/gui/x/xlib-tutorial
 *   http://user.xmission.com/~georgeps/documentation/tutorials/
 *      Xlib_Beginner.html
 */
bool X11_Init() {
  Atom deleteWindow;

  /* Open a connection to the X server */
  X11_Display = XOpenDisplay(NULL);
  if(X11_Display == NULL) {
    fprintf(stderr, "ERROR: could not open X display\n");
    return false;
  }
  X11_Screen = DefaultScreen(X11_Display);
  X11_Window = XCreateSimpleWindow(
      X11_Display,
      RootWindow(X11_Display, X11_Screen),
      0,
      0,
      ENVIRONMENT_WIDTH * PARTICLE_WIDTH,
      ENVIRONMENT_HEIGHT * PARTICLE_HEIGHT,
      1,
      BlackPixel(X11_Display, X11_Screen),
      WhitePixel(X11_Display, X11_Screen));
  deleteWindow = XInternAtom(X11_Display, "WM_DELETE_WINDOW", 0);
  XSetWMProtocols(
      X11_Display,
      X11_Window,
      &deleteWindow,
      1);
  XSelectInput(X11_Display, X11_Window, ExposureMask | KeyPressMask);
  XMapWindow(X11_Display, X11_Window);
  X11_ColorMap = DefaultColormap(X11_Display, 0);
  X11_GC = XCreateGC(X11_Display, X11_Window, 0, 0);
  XParseColor(X11_Display, X11_ColorMap, FREE_COLOR, &X11_FreeColor);
  XParseColor(X11_Display, X11_ColorMap, STUCK_COLOR, &X11_StuckColor);
  XAllocColor(X11_Display, X11_ColorMap, &X11_FreeColor);
  XAllocColor(X11_Display, X11_ColorMap, &X11_StuckColor);

  return true;
}

void X11_Finalize() {
  XDestroyWindow(X11_Display, X11_Window);
  XCloseDisplay(X11_Display);
}

void draw() {
  int i;

  XClearWindow(X11_Display, X11_Window);

  for (i = 0; i < NUM_PARTICLES; i++) {
    if (ParticleStates[i] == FREE) {
      XSetForeground(X11_Display, X11_GC, X11_FreeColor.pixel);
    }
    else {
      XSetForeground(X11_Display, X11_GC, X11_StuckColor.pixel);
    }
    XFillRectangle(X11_Display, X11_Window, X11_GC,
        ParticleXs[i] * PARTICLE_WIDTH, ParticleYs[i] * PARTICLE_HEIGHT,
        PARTICLE_WIDTH, PARTICLE_HEIGHT);
  }

  XFlush(X11_Display);
}

void getStuck() {
  int i;
  int j;

  for (i = ParticleDispls[MPIRank];
      i < ParticleDispls[MPIRank] + ParticleCounts[MPIRank]; i++) {
    for (j = 0; ParticleStates[i] == FREE && j < NUM_PARTICLES; j++) {
      if (ParticleStates[j] == STUCK &&
          ParticleXs[i] <= ParticleXs[j] + 1 &&
          ParticleXs[j] <= ParticleXs[i] + 1 &&
          ParticleYs[i] <= ParticleYs[j] + 1 &&
          ParticleYs[j] <= ParticleYs[i] + 1) {
        if (100 * random() / RAND_MAX < Stickiness) {
          ParticleStates[i] = STUCK;
        }
      }
    }
  }
}

void moveRandom() {
  int i;
  int dir;

  for (i = ParticleDispls[MPIRank];
      i < ParticleDispls[MPIRank] + ParticleCounts[MPIRank]; i++) {
    if (ParticleStates[i] == FREE) {

      /* Get random direction */
      dir = 4 * random() / RAND_MAX;

      /* Check for invalid moves into walls */
      if ((dir == UP && ParticleYs[i] <= 0) ||
          (dir == LEFT && ParticleXs[i] <= 0) ||
          (dir == RIGHT && ParticleXs[i] >= ENVIRONMENT_WIDTH - 1) ||
          (dir == DOWN && ParticleYs[i] >= ENVIRONMENT_HEIGHT - 1)) {
        dir = STAY;
      }

      /* Commit the move */
      if (dir == UP) {
        ParticleYs[i]--;
      }
      else if (dir == LEFT) {
        ParticleXs[i]--;
      }
      else if (dir == RIGHT) {
        ParticleXs[i]++;
      }
      else if (dir == DOWN) {
        ParticleYs[i]++;
      }
    }
  }
}

/* Wrapper around MPI_Allgatherv since all of the communications in
 * this program have similar, repetitive structures. */
void allgather(int *buf) {
  MPI_Allgatherv(&(buf[ParticleDispls[MPIRank]]),
      ParticleCounts[MPIRank], MPI_INT, buf, ParticleCounts, ParticleDispls,
      MPI_INT, MPI_COMM_WORLD);
}

/* Wrapper for avoiding duplicate code - performs Allgathers for all particle
 * data */
void synchronize() {
  allgather(ParticleXs);
  allgather(ParticleYs);
  allgather(ParticleStates);
}

void simulate() {
  while (RunForever || CurTime < NumTimeSteps) {

    synchronize();
    if (MPIRank == 0) {
      draw();
      usleep(IntervalUsecs);
    }
    else {
      getStuck();
    }

    synchronize();
    if (MPIRank != 0) {
      moveRandom();
    }

    CurTime++;
  }
}

void distributeWork() {
  const int evenSplit = NUM_PARTICLES / (MPISize - 1);
  const int numProcsWith1Extra = NUM_PARTICLES % (MPISize - 1);
  int i;

  ParticleCounts = (int*)malloc(MPISize * sizeof(int));
  ParticleDispls = (int*)malloc(MPISize * sizeof(int));

  /* rank 0 should not be sending particles */
  ParticleCounts[0] = 0;
  ParticleDispls[0] = 0;

  for (i = 1; i < MPISize; i++) {
    ParticleCounts[i] = evenSplit;
    /* the first few processes have one extra particle to take care of the
     * remainder */
    if (i <= numProcsWith1Extra) {
      ParticleCounts[i]++;
    }
    ParticleDispls[i] = ParticleDispls[i-1] + ParticleCounts[i-1];
  }
}

void initialize() {
  bool ok = true;
  int i;

  if (MPIRank == 0) {
    ok = X11_Init();
  }
  if (!ok) {
    MPI_Finalize();
    exit(EXIT_FAILURE);
  }

  distributeWork();

  if (MPIRank != 0) {
    srandom(RandomSeed * MPISize + MPIRank);

    if (MPIRank == 1) {
      ParticleXs[0] = ENVIRONMENT_WIDTH / 2 - 1;
      ParticleYs[0] = ENVIRONMENT_WIDTH / 2 - 1;
      ParticleStates[0] = STUCK;
      i = 1;
    }
    else {
      i = ParticleDispls[MPIRank];
    }

    for (; i < ParticleDispls[MPIRank] + ParticleCounts[MPIRank]; i++) {
      ParticleXs[i] = ENVIRONMENT_WIDTH * random() / RAND_MAX;
      ParticleYs[i] = ENVIRONMENT_HEIGHT * random() / RAND_MAX;
      ParticleStates[i] = FREE;
    }
  }
}

void printUsage(char **argv) {
  fprintf(stderr, "Usage: ");
  fprintf(stderr, "mpirun -np N %s\n", argv[0]);
  fprintf(stderr, "\t[-s Stickiness]\n");
  fprintf(stderr, "\t[-m IntervalUsecs]\n");
  fprintf(stderr, "\t[-r RandomSeed]\n");
  fprintf(stderr, "\t[-t NumTimeSteps]\n");
  fprintf(stderr, "\tN must be greater than 1.\n");
}

void getUserArgs(int argc, char **argv) {
  char c;
  float tmp;

  while ((c = getopt(argc, argv, "s:m:r:t:")) != -1) {
    switch(c) {
      case 's':
        tmp = atof(optarg);
        Stickiness = (int)tmp;
        if (tmp != Stickiness || Stickiness < 0 || Stickiness > 100) {
          if (MPIRank == 0) {
            printUsage(argv);
            fprintf(stderr,
                "ERROR: value for -s must be integer between 0 and ");
            fprintf(stderr, "100, inclusive.\n");
          }
          MPI_Finalize();
          exit(EXIT_FAILURE);
        }
        break;
      case 'm':
        tmp = atof(optarg);
        IntervalUsecs = (int)tmp;
        if (tmp != IntervalUsecs || IntervalUsecs < 1) {
          if (MPIRank == 0) {
            printUsage(argv);
            fprintf(stderr, "ERROR: value for -m must be positive integer.\n");
          }
          MPI_Finalize();
          exit(EXIT_FAILURE);
        }
        break;
      case 'r':
        tmp = atof(optarg);
        RandomSeed = (int)tmp;
        if (tmp != RandomSeed || RandomSeed < 1) {
          if (MPIRank == 0) {
            printUsage(argv);
            fprintf(stderr, "ERROR: value for -r must be positive integer.\n");
          }
          MPI_Finalize();
          exit(EXIT_FAILURE);
        }
        break;
      case 't':
        tmp = atof(optarg);
        NumTimeSteps = (int)tmp;
        RunForever = false;
        if (tmp != NumTimeSteps || NumTimeSteps < 1) {
          if (MPIRank == 0) {
            printUsage(argv);
            fprintf(stderr, "ERROR: value for -t must be positive integer.\n");
          }
          MPI_Finalize();
          exit(EXIT_FAILURE);
        }
        break;
      case '?':
      default:
        if (MPIRank == 0) {
          printUsage(argv);
        }
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }
  }
  argc -= optind;
  argv += optind;
}

int main(int argc, char **argv) {
  /* Initialize MPI */
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &MPIRank);
  MPI_Comm_size(MPI_COMM_WORLD, &MPISize);

  /* Make sure there are at least 2 MPI processes */
  if (MPISize < 2) {
    if (MPIRank == 0) {
      printUsage(argv);
      fprintf(stderr,
          "ERROR: Must run with at least 2 MPI processes.\n");
    }
    MPI_Finalize();
    exit(EXIT_FAILURE);
  }

  RandomSeed = time(NULL);

  getUserArgs(argc, argv);
  initialize();
  simulate();
  if (MPIRank == 0) {
    X11_Finalize();
  }

  free(ParticleDispls);
  free(ParticleCounts);

  MPI_Finalize();

  return 0;
}

