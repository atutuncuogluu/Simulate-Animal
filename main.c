#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>


typedef enum
{
    BEAR,
    BIRD,
    PANDA
} AnimalType;
typedef enum
{
    ALIVE,
    DEAD
} AnimalStatus;

typedef struct
{
    int x;
    int y;
} Location;

typedef enum
{
    FEEDING,
    NESTING,
    WINTERING
} SiteType;

typedef struct
{
    AnimalStatus status;
    AnimalType type;
    Location location;
} Animal;

Animal bird, bear, panda;

typedef struct
{
    int points;
    Location location;
} Hunter;

typedef struct
{
    Hunter **hunters;
    int nhunters;
    Animal **animals;
    int nanimals;
    SiteType type;
} Site;

/** 2D site grid */
typedef struct
{
    int xlength;
    int ylength;
    Site **sites;
} Grid;

/* initial grid, empty */
Grid grid = {0, 0, NULL};

Grid initgrid(int xlength, int ylength)
{
    grid.xlength = xlength;
    grid.ylength = ylength;

    // Seed the random number generator with the current time
    srand(time(NULL));

    grid.sites = (Site **)malloc(sizeof(Site *) * xlength);
    for (int i = 0; i < xlength; i++)
    {
        grid.sites[i] = (Site *)malloc(sizeof(Site) * ylength);
        for (int j = 0; j < ylength; j++)
        {
            grid.sites[i][j].animals = NULL;
            grid.sites[i][j].hunters = NULL;
            grid.sites[i][j].nhunters = 0;
            grid.sites[i][j].nanimals = 0;
            double r = rand() / (double)RAND_MAX;
            SiteType st;
            if (r < 0.33)
                st = WINTERING;
            else if (r < 0.66)
                st = FEEDING;
            else
                st = NESTING;
            grid.sites[i][j].type = st;
        }
    }

    return grid;
}

void deletegrid()
{
    for (int i = 0; i < grid.xlength; i++)
    {
        free(grid.sites[i]);
    }

    free(grid.sites);

    grid.sites = NULL;
    grid.xlength = -1;
    grid.ylength = -1;
}

void printgrid()
{
    for (int i = 0; i < grid.xlength; i++)
    {
        for (int j = 0; j < grid.ylength; j++)
        {
            Site *site = &grid.sites[i][j];
            int count[3] = {0}; /* do not forget to initialize*/
            for (int a = 0; a < site->nanimals; a++)
            {
                Animal *animal = site->animals[a];
                count[animal->type]++;
            }

            printf("|%d-{%d, %d, %d}{%d}|", site->type, count[0], count[1],
                   count[2], site->nhunters);
        }
        printf("\n");
    }
}

void printsite(Site *site)
{
    int count[3] = {0}; /* do not forget to initialize*/
    for (int a = 0; a < site->nanimals; a++)
    {
        Animal *animal = site->animals[a];
        count[animal->type]++;
    }
    printf("|%d-{%d,%d,%d}{%d}|", site->type, count[0], count[1], count[2],
           site->nhunters);
}

void *simulateanimal(void *args)
{
    Animal *animal = (Animal *)args;

    while (1)
    {
        // Get the current site location of the animal
        int currentX = animal->location.x;
        int currentY = animal->location.y;
        SiteType currentSiteType = grid.sites[currentX][currentY].type;

        // Simulate behavior based on the current site type
        if (currentSiteType == NESTING)
        {
            // Create a new animal of the same type at the current site
            Animal *newAnimal = (Animal *)malloc(sizeof(Animal));
            newAnimal->status = ALIVE;
            newAnimal->type = animal->type;
            newAnimal->location.x = currentX;
            newAnimal->location.y = currentY;

            // Add the new animal to the current site
            Site *currentSite = &grid.sites[currentX][currentY];
            currentSite->animals = (Animal **)realloc(currentSite->animals, sizeof(Animal *) * (currentSite->nanimals + 1));
            currentSite->animals[currentSite->nanimals] = newAnimal;
            currentSite->nanimals++;

            // Move the current animal to a neighboring random location
            int newX, newY;
            do
            {
                newX = currentX + (rand() % 3) - 1;
                newY = currentY + (rand() % 3) - 1;
            } while (newX < 0 || newX >= grid.xlength || newY < 0 || newY >= grid.ylength || (newX == currentX && newY == currentY));

            // Update the location of the current animal
            animal->location.x = newX;
            animal->location.y = newY;
        }
        else if (currentSiteType == WINTERING)
        {
            // Check if the animal dies with 0.5 probability
            if ((rand() / (double)RAND_MAX) <= 0.5)
            {
                animal->status = DEAD;
                break;
            }

            // Move the current animal to a neighboring random location
            int newX, newY;
            do
            {
                newX = currentX + (rand() % 3) - 1;
                newY = currentY + (rand() % 3) - 1;
            } while (newX < 0 || newX >= grid.xlength || newY < 0 || newY >= grid.ylength || (newX == currentX && newY == currentY));

            // Update the location of the current animal
            animal->location.x = newX;
            animal->location.y = newY;
        }
        else if (currentSiteType == FEEDING)
        {
            // Check if the animal moves with 0.8 probability
            if ((rand() / (double)RAND_MAX) > 0.8)
            {
                // Move the current animal to a neighboring random location
                int newX, newY;
                do
                {
                    newX = currentX + (rand() % 3) - 1;
                    newY = currentY + (rand() % 3) - 1;
                } while (newX < 0 || newX >= grid.xlength || newY < 0 || newY >= grid.ylength || (newX == currentX && newY == currentY));

                // Update the location of the current animal
                animal->location.x = newX;
                animal->location.y = newY;
            }
        }

        // Delay before the next simulation time unit
        usleep(100000); // Adjust the delay time as needed
    }

    return NULL;
}

void *simulatehunter(void *args)
{
    Hunter *hunter = (Hunter *)args;

    while (1)
    {
        // Get the current site location of the hunter
        int currentX = hunter->location.x;
        int currentY = hunter->location.y;

        // Move the hunter to a neighboring random location
        int newX, newY;
        do
        {
            newX = currentX + (rand() % 3) - 1;
            newY = currentY + (rand() % 3) - 1;
        } while (newX < 0 || newX >= grid.xlength || newY < 0 || newY >= grid.ylength || (newX == currentX && newY == currentY));

        // Update the location of the hunter
        hunter->location.x = newX;
        hunter->location.y = newY;

        // Check if there are animals at the new location and kill them
        Site *currentSite = &grid.sites[newX][newY];
        int numKills = currentSite->nanimals;
        currentSite->nanimals = 0;
        free(currentSite->animals);
        currentSite->animals = NULL;

        // Update the hunter's points based on the number of kills
        hunter->points += numKills;

        // Delay before the next movement
        usleep(100000); // Adjust the delay time as needed
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s <number of hunters>\n", argv[0]);
        return 1;
    }

    int numHunters = atoi(argv[1]);

    initgrid(3, 3);

    // Create thread for bear
    pthread_t bear_thread;
    pthread_create(&bear_thread, NULL, simulateanimal, (void *)&bear);

    // Create thread for panda
    pthread_t panda_thread;
    pthread_create(&panda_thread, NULL, simulateanimal, (void *)&panda);

    // Create thread for bird
    pthread_t bird_thread;
    pthread_create(&bird_thread, NULL, simulateanimal, (void *)&bird);

    // Create threads for hunters
    pthread_t hunter_threads[numHunters];
    for (int i = 0; i < numHunters; i++)
    {
        Hunter *hunter = (Hunter *)malloc(sizeof(Hunter));
        hunter->points = 0;

        // Set the initial location of the hunter randomly
        int initialX, initialY;
        do
        {
            initialX = rand() % grid.xlength;
            initialY = rand() % grid.ylength;
        } while (grid.sites[initialX][initialY].nhunters > 0 || grid.sites[initialX][initialY].nanimals > 0);

        hunter->location.x = initialX;
        hunter->location.y = initialY;

        // Add the hunter to the site
        grid.sites[initialX][initialY].hunters = (Hunter **)realloc(grid.sites[initialX][initialY].hunters, sizeof(Hunter *) * (grid.sites[initialX][initialY].nhunters + 1));
        grid.sites[initialX][initialY].hunters[grid.sites[initialX][initialY].nhunters] = hunter;
        grid.sites[initialX][initialY].nhunters++;

        // Create the hunter thread
        pthread_create(&hunter_threads[i], NULL, simulatehunter, (void *)hunter);
    }

    // Run the simulation for 1 second
    usleep(1000000); // Sleep for 1 second

    // Cancel animal threads
    pthread_cancel(bear_thread);
    pthread_cancel(panda_thread);
    pthread_cancel(bird_thread);

    // Wait for animal threads to finish
    void *threadResult;
    pthread_join(bear_thread, &threadResult);
    pthread_join(panda_thread, &threadResult);
    pthread_join(bird_thread, &threadResult);

    // Cancel hunter threads
    for (int i = 0; i < numHunters; i++)
    {
        pthread_cancel(hunter_threads[i]);
    }

    // Wait for hunter threads to finish
    for (int i = 0; i < numHunters; i++)
    {
        pthread_join(hunter_threads[i], &threadResult);
    }
    printgrid();
    deletegrid();

    return 0;
}
