#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct TransactionNo
typedef struct DayRecordNode
{
    int date;
    float budget;
    float remaining;
    struct DayRecordNode *next;
} DayRecordNode;

DayRecordNode *createMonthRecord(float monthlyBudget, int daysInMonth)
{
    DayRecordNode *head = NULL, *temp, *newNode;
    float dailyBudget = monthlyBudget / daysInMonth;

    for (int i = 1; i <= daysInMonth; i++)
    {
        newNode = (DayRecordNode *)malloc(sizeof(DayRecordNode));
        newNode->date = i;
        newNode->budget = dailyBudget;
        newNode->remaining = dailyBudget;
        newNode->next = NULL;

        if (head == NULL)
        {
            head = newNode;
        }
        else
        {
            temp->next = newNode;
        }
        temp = newNode;
    }
    return head;
}

void addTransaction(DayRecordNode *monthlyRecord, int date, float amount, char *category, int daysInMonth)
{
    DayRecordNode *dayRecord = monthlyRecord;

    while (dayRecord != NULL && dayRecord->date != date)
    {
        dayRecord = dayRecord->next;
    }

    if (dayRecord == NULL)
    {
        printf("Invalid date!\n");
        return;
    }

    TransactionNode *newTransaction = (TransactionNode *)malloc(sizeof(TransactionNode));
    newTransaction->date = date;
    newTransaction->amount = amount;
    strcpy(newTransaction->category, category);
    newTransaction->next = NULL;

    float difference = dayRecord->remaining - amount;
    if (difference >= 0)
    {
        dayRecord->remaining = difference;
    }
    else
    {
        dayRecord->remaining = 0;
        float deficitPerDay = -difference / (daysInMonth - date);
        DayRecordNode *temp = dayRecord->next;
        while (temp != NULL)
        {
            temp->remaining -= deficitPerDay;
            temp = temp->next;
        }
    }
}

void displayRemainingBudget(DayRecordNode *monthlyRecord, int date)
{
    DayRecordNode *dayRecord = monthlyRecord;

    while (dayRecord != NULL && dayRecord->date != date)
    {
        dayRecord = dayRecord->next;
    }

    if (dayRecord == NULL)
    {
        printf("Invalid date!\n");
    }
    else
    {
        printf("Remaining budget for %d: %.2f\n", date, dayRecord->remaining);
    }
}

void saveDataToFile(DayRecordNode *monthlyRecord, char *filename)
{
    FILE *fp = fopen(filename, "w");
    if (fp == NULL)
    {
        printf("Error opening file!\n");
        return;
    }

    DayRecordNode *dayRecord = monthlyRecord;
    TransactionNode *transaction;

    while (dayRecord != NULL)
    {
        fprintf(fp, "%d,%.2f,%.2f\n", dayRecord->date, dayRecord->budget, dayRecord->remaining);
        dayRecord = dayRecord->next;
    }

    fclose(fp);
    printf("Data saved successfully!\n");
}

DayRecordNode *loadDataFromFile(char *filename)
{
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("Error opening file or file not found!\n");
        return NULL;
    }

    DayRecordNode *head = NULL, *temp, *newNode;
    int date;
    float budget, remaining;

    while (fscanf(fp, "%d,%f,%f", &date, &budget, &remaining) == 3)
    {
        newNode = (DayRecordNode *)malloc(sizeof(DayRecordNode));
        newNode->date = date;
        newNode->budget = budget;
        newNode->remaining = remaining;
        newNode->next = NULL;

        if (head == NULL)
        {
            head = newNode;
        }
        else
        {
            temp->next = newNode;
        }
        temp = newNode;
    }

    fclose(fp);
    return head;
}

int main()
{
    float monthlyBudget;
    int daysInMonth, choice, date;
    float amount;
    char category[50];
    char filename[50] = "budget_data.csv";

    printf("Enter monthly budget: ");
    scanf("%f", &monthlyBudget);

    printf("Enter number of days in the month: ");
    scanf("%d", &daysInMonth);

    DayRecordNode *monthlyRecord = createMonthRecord(monthlyBudget, daysInMonth);

    do
    {
        printf("\nMenu:\n");
        printf("1. Add Transaction\n");
        printf("2. View Remaining Budget\n");
        printf("3. Save Data\n");
        printf("4. Load Data\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice)
        {
        case 1:
            printf("Enter date (1-%d): ", daysInMonth);
            scanf("%d", &date);
            printf("Enter amount: ");
            scanf("%f", &amount);
            printf("Enter category: ");
            scanf("%s", category);
            addTransaction(monthlyRecord, date, amount, category, daysInMonth);
            break;
        case 2:
            printf("Enter date (1-%d): ", daysInMonth);
            scanf("%d", &date);
            displayRemainingBudget(monthlyRecord, date);
            break;
        case 3:
            saveDataToFile(monthlyRecord, filename);
            break;
        case 4:
            monthlyRecord = loadDataFromFile(filename);
            if (monthlyRecord == NULL)
            {
                printf("No saved data found.\n");
            }
            break;
        case 5:
            printf("Exiting...\n");
            break;
        default:
            printf("Invalid choice!\n");
        }
    } while (choice != 5);

    saveDataToFile(monthlyRecord, filename);

    return 0;
}
