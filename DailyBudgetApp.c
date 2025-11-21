#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct TransactionNode {
    int date;
    float amount;
    char category[50];
    struct TransactionNode *next;
} TransactionNode;

typedef struct DayRecordNode {
    int date;
    float budget;
    float remaining;
    TransactionNode *transactions; // attach transactions to each day
    struct DayRecordNode *next;
} DayRecordNode;

// Create monthly record with daily budgets
DayRecordNode *createMonthRecord(float monthlyBudget, int daysInMonth) {
    DayRecordNode *head = NULL, *temp = NULL, *newNode;
    float dailyBudget = monthlyBudget / daysInMonth;

    for (int i = 1; i <= daysInMonth; i++) {
        newNode = (DayRecordNode *)malloc(sizeof(DayRecordNode));
        newNode->date = i;
        newNode->budget = dailyBudget;
        newNode->remaining = dailyBudget;
        newNode->transactions = NULL;
        newNode->next = NULL;

        if (head == NULL) {
            head = newNode;
        } else {
            temp->next = newNode;
        }
        temp = newNode;
    }
    return head;
}

// Add transaction to a specific day
void addTransaction(DayRecordNode *monthlyRecord, int date, float amount, char *category, int daysInMonth) {
    DayRecordNode *dayRecord = monthlyRecord;

    while (dayRecord != NULL && dayRecord->date != date) {
        dayRecord = dayRecord->next;
    }

    if (dayRecord == NULL) {
        printf("Invalid date!\n");
        return;
    }

    TransactionNode *newTransaction = (TransactionNode *)malloc(sizeof(TransactionNode));
    newTransaction->date = date;
    newTransaction->amount = amount;
    strncpy(newTransaction->category, category, sizeof(newTransaction->category) - 1);
    newTransaction->category[sizeof(newTransaction->category) - 1] = '\0';
    newTransaction->next = dayRecord->transactions;
    dayRecord->transactions = newTransaction;

    float difference = dayRecord->remaining - amount;
    if (difference >= 0) {
        dayRecord->remaining = difference;
    } else {
        dayRecord->remaining = 0;
        if (daysInMonth - date > 0) { // avoid division by zero
            float deficitPerDay = -difference / (daysInMonth - date);
            DayRecordNode *temp = dayRecord->next;
            while (temp != NULL) {
                temp->remaining -= deficitPerDay;
                if (temp->remaining < 0) temp->remaining = 0;
                temp = temp->next;
            }
        }
    }
}

// Display remaining budget for a given day
void displayRemainingBudget(DayRecordNode *monthlyRecord, int date) {
    DayRecordNode *dayRecord = monthlyRecord;
    while (dayRecord != NULL && dayRecord->date != date) {
        dayRecord = dayRecord->next;
    }
    if (dayRecord == NULL) {
        printf("Invalid date!\n");
    } else {
        printf("Remaining budget for day %d: %.2f\n", date, dayRecord->remaining);
    }
}

// Display all transactions for a given day
void displayTransactions(DayRecordNode *monthlyRecord, int date) {
    DayRecordNode *dayRecord = monthlyRecord;
    while (dayRecord != NULL && dayRecord->date != date) {
        dayRecord = dayRecord->next;
    }
    if (dayRecord == NULL) {
        printf("Invalid date!\n");
        return;
    }
    printf("Transactions for day %d:\n", date);
    TransactionNode *t = dayRecord->transactions;
    if (t == NULL) {
        printf("  No transactions recorded.\n");
    }
    while (t != NULL) {
        printf("  %.2f spent on %s\n", t->amount, t->category);
        t = t->next;
    }
}

// Display monthly summary
void displayMonthlySummary(DayRecordNode *monthlyRecord) {
    float totalSpent = 0, totalBudget = 0;
    DayRecordNode *dayRecord = monthlyRecord;

    while (dayRecord != NULL) {
        totalBudget += dayRecord->budget;
        TransactionNode *t = dayRecord->transactions;
        while (t != NULL) {
            totalSpent += t->amount;
            t = t->next;
        }
        dayRecord = dayRecord->next;
    }
    printf("\nMonthly Summary:\n");
    printf("Total Budget: %.2f\n", totalBudget);
    printf("Total Spent: %.2f\n", totalSpent);
    printf("Savings: %.2f\n", totalBudget - totalSpent);
}

// Free memory
void freeMonthlyRecord(DayRecordNode *monthlyRecord) {
    while (monthlyRecord != NULL) {
        TransactionNode *t = monthlyRecord->transactions;
        while (t != NULL) {
            TransactionNode *tmpT = t;
            t = t->next;
            free(tmpT);
        }
        DayRecordNode *tmp = monthlyRecord;
        monthlyRecord = monthlyRecord->next;
        free(tmp);
    }
}

int main() {
    float monthlyBudget;
    int daysInMonth, choice, date;
    float amount;
    char category[50];

    printf("Enter monthly budget: ");
    scanf("%f", &monthlyBudget);

    printf("Enter number of days in the month: ");
    scanf("%d", &daysInMonth);

    DayRecordNode *monthlyRecord = createMonthRecord(monthlyBudget, daysInMonth);

    do {
        printf("\nMenu:\n");
        printf("1. Add Transaction\n");
        printf("2. View Remaining Budget\n");
        printf("3. View Transactions for a Day\n");
        printf("4. View Monthly Summary\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
        case 1:
            printf("Enter date (1-%d): ", daysInMonth);
            scanf("%d", &date);
            printf("Enter amount: ");
            scanf("%f", &amount);
            printf("Enter category: ");
            scanf("%49s", category);
            addTransaction(monthlyRecord, date, amount, category, daysInMonth);
            break;
        case 2:
            printf("Enter date (1-%d): ", daysInMonth);
            scanf("%d", &date);
            displayRemainingBudget(monthlyRecord, date);
            break;
        case 3:
            printf("Enter date (1-%d): ", daysInMonth);
            scanf("%d", &date);
            displayTransactions(monthlyRecord, date);
            break;
        case 4:
            displayMonthlySummary(monthlyRecord);
            break;
        case 5:
            printf("Exiting...\n");
            break;
        default:
            printf("Invalid choice!\n");
        }
    } while (choice != 5);

    freeMonthlyRecord(monthlyRecord);
    return 0;
}
