#include <cs50.h>
#include <stdio.h>
#include <string.h>

// Max number of candidates
#define MAX 9

// preferences[i][j] is number of voters who prefer i over j
int preferences[MAX][MAX];

// locked[i][j] means i is locked in over j
bool locked[MAX][MAX];

// Each pair has a winner, loser
typedef struct
{
    int winner;
    int loser;
}
pair;

// Array of candidates
string candidates[MAX];
pair pairs[MAX * (MAX - 1) / 2];

int pair_count;
int candidate_count;

// Function prototypes
bool vote(int rank, string name, int ranks[]);
void record_preferences(int ranks[]);
void add_pairs(void);
void sort_pairs(void);
void lock_pairs(void);
void print_winner(void);

int main(int argc, string argv[])
{
    // Check for invalid usage
    if (argc < 2)
    {
        printf("Usage: tideman [candidate ...]\n");
        return 1;
    }

    // Populate array of candidates
    candidate_count = argc - 1;
    if (candidate_count > MAX)
    {
        printf("Maximum number of candidates is %i\n", MAX);
        return 2;
    }
    for (int i = 0; i < candidate_count; i++)
    {
        candidates[i] = argv[i + 1];
    }

    // Clear graph of locked in pairs
    for (int i = 0; i < candidate_count; i++)
    {
        for (int j = 0; j < candidate_count; j++)
        {
            locked[i][j] = false;
        }
    }

    pair_count = 0;
    int voter_count = get_int("Number of voters: ");

    // Query for votes
    for (int i = 0; i < voter_count; i++)
    {
        // ranks[i] is voter's ith preference
        int ranks[candidate_count];

        // Query for each rank
        for (int j = 0; j < candidate_count; j++)
        {
            string name = get_string("Rank %i: ", j + 1);

            if (!vote(j, name, ranks))
            {
                printf("Invalid vote.\n");
                return 3;
            }
        }

        record_preferences(ranks);

        printf("\n");
    }

    add_pairs();
    sort_pairs();
    lock_pairs();
    print_winner();
    return 0;
}

// Update ranks given a new vote
bool vote(int rank, string name, int ranks[])
{
    for (int i = 0; i < candidate_count; i++)
    {
        if (strcmp(candidates[i], name) == 0)
        {
            ranks[rank] = i;
            return true;
        }
    }
    return false;
}

// Debug routine to dump preferences
void dumpPreferences()
{
    for (int i = 0; i < candidate_count; i++)
    {
        printf("%s:", candidates[i]);
        for (int j = 0; j < candidate_count; j++)
        {
            if ((i != j) && (preferences[i][j] != 0))
            {
                printf(" %s(%d)", candidates[j], preferences[i][j]);
            }
        }
        printf("\n");
    }
}

// Update preferences given one voter's ranks
void record_preferences(int ranks[])
{
    for (int i = 0; i < candidate_count - 1; i++)
    {
        for (int j = i + 1; j < candidate_count; j++)
        {
            preferences[ranks[i]][ranks[j]]++;
        }
    }
    // dumpPreferences();
    return;
}

// A debug routine to see the pairs
void dumpPairs(string message)
{
    printf("dumpPairs(%s): (%i)\n", message, pair_count);
    for (int i = 0; i < pair_count; i++)
    {
        printf("  pair: %s, %s\n", candidates[pairs[i].winner], candidates[pairs[i].loser]);
    }
}

// Record pairs of candidates where one is preferred over the other
void add_pairs(void)
{
    for (int i = 0; i < candidate_count - 1; i++)
    {
        for (int j = i; j < candidate_count; j++)
        {
            if (preferences[i][j] != preferences[j][i])
            {
                if (preferences[i][j] > preferences[j][i])
                {
                    pairs[pair_count].winner = i;
                    pairs[pair_count].loser = j;
                }
                else
                {
                    pairs[pair_count].winner = j;
                    pairs[pair_count].loser = i;
                }
                pair_count++;
            }
        }
    }
    // dumpPairs("DumpPairs");
    return;
}

// Swap two pair[] entries while sorting
void swapPairs(int first, int second)
{
    int tempWinner = pairs[first].winner;
    int tempLoser = pairs[first].loser;
    pairs[first].winner = pairs[second].winner;
    pairs[first].loser = pairs[second].loser;
    pairs[second].winner = tempWinner;
    pairs[second].loser = tempLoser;
}

// Perform a recursive merge sort on the pair[]'s in reverse order
void mergeSortPairs(int low, int high)
{
    if (low >= high)
    {
        return;
    }
    if (low + 1 == high)
    {
        if (preferences[pairs[low].winner][pairs[low].loser] < preferences[pairs[high].winner][pairs[high].loser])
        {
            // printf("  swapPairs(%i,%i)\n", low, high);
            swapPairs(low, high);
        }
        return;
    }

    // find the mid-point and sort left half then right half
    int middle = (high + low) / 2;
    mergeSortPairs(low, middle);
    mergeSortPairs(middle + 1, high);

    // merge the two halves into a temp array
    pair tempPairs[high - low + 1];
    int tempPos = 0;
    int leftPos = low;
    int rightPos = middle + 1;
    while (leftPos <= middle || rightPos <= high)
    {
        if (rightPos > high)
        {
            tempPairs[tempPos].winner = pairs[leftPos].winner;
            tempPairs[tempPos].loser = pairs[leftPos].loser;
            tempPos++;
            leftPos++;
        }
        else if (leftPos > middle)
        {
            tempPairs[tempPos].winner = pairs[rightPos].winner;
            tempPairs[tempPos].loser = pairs[rightPos].loser;
            tempPos++;
            rightPos++;
        }
        else
        {
            if (preferences[pairs[leftPos].winner][pairs[leftPos].loser] > preferences[pairs[rightPos].winner][pairs[rightPos].loser])
            {
                tempPairs[tempPos].winner = pairs[leftPos].winner;
                tempPairs[tempPos].loser = pairs[leftPos].loser;
                tempPos++;
                leftPos++;
            }
            else
            {
                tempPairs[tempPos].winner = pairs[rightPos].winner;
                tempPairs[tempPos].loser = pairs[rightPos].loser;
                tempPos++;
                rightPos++;
            }
        }
    }

    // place the merged temp array back into the orginal
    for (int i = low; i <= high; i++)
    {
        pairs[i].winner = tempPairs[i].winner;
        pairs[i].loser = tempPairs[i].loser;
    }
}

// Sort pairs in decreasing order by strength of victory
void sort_pairs(void)
{
    if (pair_count < 2)
    {
        // Can't really sort pairs unless there are multiple pairs
        return;
    }

    mergeSortPairs(0, pair_count - 1);
    // dumpPairs("SortedPairs");
    return;
}

// use recursion to try all possible paths and see if we can find one
bool findPathFromTo(int from, int to)
{
    if (locked[from][to])
    {
        return true;
    }

    for (int i = 0; i < candidate_count; i++)
    {
        if (locked[from][i])
        {
            if (findPathFromTo(i, to))
            {
                return true;
            }
        }
    }

    return false;
}

// Start out with the assumption no cycle currently exists and see if
// making a proposed addition would create a cycle, return true if it
// would, false otherwise
bool wouldMakeCycle(int winner, int loser)
{
    // Already present, how did that happen?
    if (locked[winner][loser])
    {
        return false;
    }

    // temporarily put the entry in before looking for a cycle
    locked[winner][loser] = true;

    // Start with the first entry because we know it was there
    bool result = findPathFromTo(winner, winner);

    // undo the temp change
    locked[winner][loser] = false;

    return result;
}

// Lock pairs into the candidate graph in order, without creating cycles
void lock_pairs(void)
{
    // adding the first entry CANNOT make a cycle
    locked[pairs[0].winner][pairs[0].loser] = true;

    if (pair_count < 2)
    {
        // Can't do more if only one pair
        return;
    }

    for (int i = 1; i < pair_count; i++)
    {
        if (!wouldMakeCycle(pairs[i].winner, pairs[i].loser))
        {
            locked[pairs[i].winner][pairs[i].loser] = true;
        }
    }
    return;
}

// get the number of candidates in the graph that point at the
// requested candidate
bool getNumberPointingAtCandidate(int candidate)
{
    int result = 0;
    for (int i = 0; i < candidate_count; i++)
    {
        if (locked[i][candidate])
        {
            result++;
        }
    }
    return result;
}

// Print the winner of the election
void print_winner(void)
{
    if (candidate_count < 2)
    {
        // Accliamed candidate
        printf("%s\n", candidates[0]);
    }
    else
    {
        for (int i = 0; i < candidate_count; i++)
        {
            if (getNumberPointingAtCandidate(i) == 0)
            {
                printf("%s\n", candidates[i]);
            }
        }
    }
    return;
}
