#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_CATEGORY_LEN 50
#define DEFAULT_PASSWORD "admin123"
#define DAYS_FILENAME "budget_data.csv"
#define TRANSACTIONS_FILENAME "transactions.csv"
#define ARCHIVE_PREFIX "budget_archive_"

// ---------- Data Structures ----------
typedef struct TransactionNode {
    float amount;
    char category[MAX_CATEGORY_LEN];
    struct TransactionNode *next;
} TransactionNode;

typedef struct DayRecordNode {
    int date;                    // 1..daysInMonth
    float budget;                // per-day budget (initial)
    float remaining;             // remaining for that day
    TransactionNode *transactions;
    struct DayRecordNode *next;
} DayRecordNode;

// ---------- Utility Functions ----------
void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

char *trim_newline(char *s) {
    if (!s) return s;
    size_t n = strlen(s);
    if (n && s[n-1] == '\n') s[n-1] = '\0';
    return s;
}

// ---------- Creation / Load / Save ----------
DayRecordNode *createMonthRecord(float monthlyBudget, int daysInMonth) {
    DayRecordNode *head = NULL, *temp = NULL;
    float perDay = monthlyBudget / (float)daysInMonth;
    for (int d = 1; d <= daysInMonth; ++d) {
        DayRecordNode *newNode = (DayRecordNode *)malloc(sizeof(DayRecordNode));
        newNode->date = d;
        newNode->budget = perDay;
        newNode->remaining = perDay;
        newNode->transactions = NULL;
        newNode->next = NULL;
        if (head == NULL) head = newNode;
        else temp->next = newNode;
        temp = newNode;
    }
    return head;
}

int passwordCheck(const char *correctPassword) {
    char pass[100];
    printf("Enter password: ");
    if (scanf("%99s", pass) != 1) { clearInputBuffer(); return 0; }
    clearInputBuffer();
    return strcmp(pass, correctPassword) == 0;
}

void saveDataToFile(DayRecordNode *monthlyRecord, const char *daysFile, const char *transFile, int daysInMonth) {
    // Save day-by-day summary
    FILE *fp = fopen(daysFile, "w");
    if (!fp) {
        printf("Error opening file %s for writing!\n", daysFile);
        return;
    }
    DayRecordNode *d = monthlyRecord;
    while (d) {
        fprintf(fp, "%d,%.2f,%.2f\n", d->date, d->budget, d->remaining);
        d = d->next;
    }
    fclose(fp);

    // Save transactions
    fp = fopen(transFile, "w");
    if (!fp) {
        printf("Error opening file %s for writing!\n", transFile);
        return;
    }
    d = monthlyRecord;
    while (d) {
        TransactionNode *t = d->transactions;
        while (t) {
            // date,amount,category
            fprintf(fp, "%d,%.2f,%s\n", d->date, t->amount, t->category);
            t = t->next;
        }
        d = d->next;
    }
    fclose(fp);

    printf("Data saved to '%s' and '%s' successfully.\n", daysFile, transFile);
}

DayRecordNode *loadDaysFromFile(const char *daysFile, int *outDaysInMonth) {
    FILE *fp = fopen(daysFile, "r");
    if (!fp) {
        // file not found
        return NULL;
    }
    DayRecordNode *head = NULL, *temp = NULL, *newNode;
    int date;
    float budget, remaining;
    int maxDate = 0;
    while (fscanf(fp, "%d,%f,%f\n", &date, &budget, &remaining) == 3) {
        newNode = (DayRecordNode *)malloc(sizeof(DayRecordNode));
        newNode->date = date;
        newNode->budget = budget;
        newNode->remaining = remaining;
        newNode->transactions = NULL;
        newNode->next = NULL;
        if (!head) head = newNode;
        else temp->next = newNode;
        temp = newNode;
        if (date > maxDate) maxDate = date;
    }
    fclose(fp);
    if (outDaysInMonth) *outDaysInMonth = maxDate;
    return head;
}

void attachTransactionsFromFile(DayRecordNode *monthlyRecord, const char *transFile) {
    FILE *fp = fopen(transFile, "r");
    if (!fp) return;
    int date;
    float amount;
    char category[MAX_CATEGORY_LEN];
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        trim_newline(line);
        // parse: date,amount,category
        char *p = line;
        char *tok = strtok(p, ",");
        if (!tok) continue;
        date = atoi(tok);
        tok = strtok(NULL, ",");
        if (!tok) continue;
        amount = atof(tok);
        tok = strtok(NULL, ",");
        if (!tok) continue;
        strncpy(category, tok, MAX_CATEGORY_LEN-1); category[MAX_CATEGORY_LEN-1]=0;

        // find corresponding day
        DayRecordNode *d = monthlyRecord;
        while (d && d->date != date) d = d->next;
        if (!d) continue;
        TransactionNode *t = (TransactionNode *)malloc(sizeof(TransactionNode));
        t->amount = amount;
        strncpy(t->category, category, MAX_CATEGORY_LEN-1); t->category[MAX_CATEGORY_LEN-1]=0;
        t->next = d->transactions;
        d->transactions = t;
    }
    fclose(fp);
}

void archiveCurrentFiles(const char *daysFile, const char *transFile) {
    // Create timestamped archive filename and rename original files if they exist
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char stamp[64];
    strftime(stamp, sizeof(stamp), "%Y%m%d_%H%M%S", t);

    char newDays[256], newTrans[256];
    snprintf(newDays, sizeof(newDays), "%s%s_%s", ARCHIVE_PREFIX, daysFile, stamp);
    snprintf(newTrans, sizeof(newTrans), "%s%s_%s", ARCHIVE_PREFIX, transFile, stamp);

    // rename only if files exist
    if (rename(daysFile, newDays) == 0) {
        printf("Archived %s -> %s\n", daysFile, newDays);
    }
    if (rename(transFile, newTrans) == 0) {
        printf("Archived %s -> %s\n", transFile, newTrans);
    }
}

// ---------- Transaction / Day Operations ----------
DayRecordNode *findDay(DayRecordNode *monthlyRecord, int date) {
    DayRecordNode *d = monthlyRecord;
    while (d && d->date != date) d = d->next;
    return d;
}

void recalculateDailyRemaining(DayRecordNode *monthlyRecord) {
    // Ensure remaining doesn't exceed budget and is non-negative.
    DayRecordNode *d = monthlyRecord;
    while (d) {
        if (d->remaining > d->budget) d->remaining = d->budget;
        if (d->remaining < 0) d->remaining = 0;
        d = d->next;
    }
}

int countRemainingDays(DayRecordNode *d) {
    int cnt = 0;
    while (d) { ++cnt; d = d->next; }
    return cnt;
}

void addTransaction(DayRecordNode *monthlyRecord, int date, float amount, const char *category, int daysInMonth, float monthlyBudget) {
    DayRecordNode *dayRecord = findDay(monthlyRecord, date);
    if (!dayRecord) {
        printf("Invalid date!\n");
        return;
    }

    // Create transaction and add to list
    TransactionNode *newTransaction = (TransactionNode *)malloc(sizeof(TransactionNode));
    newTransaction->amount = amount;
    strncpy(newTransaction->category, category, MAX_CATEGORY_LEN-1);
    newTransaction->category[MAX_CATEGORY_LEN-1] = '\0';
    newTransaction->next = dayRecord->transactions;
    dayRecord->transactions = newTransaction;

    // Deduct from this day's remaining
    float difference = dayRecord->remaining - amount;
    if (difference >= 0) {
        dayRecord->remaining = difference;
    } else {
        // deficit needs spreading across remaining days AFTER this date
        dayRecord->remaining = 0;
        DayRecordNode *temp = dayRecord->next;
        int remDays = countRemainingDays(temp);
        if (remDays > 0) {
            float deficitPerDay = -difference / (float)remDays;
            while (temp) {
                temp->remaining -= deficitPerDay;
                if (temp->remaining < 0) temp->remaining = 0;
                temp = temp->next;
            }
        } else {
            // No future days: just allow overspend (kept as zero)
        }
    }

    // After adding transaction, optionally recalc auto daily limit (for user info)
    // We'll compute remaining monthly total and split across remaining days
    DayRecordNode *d = monthlyRecord;
    float totalRemaining = 0.0f;
    int remCount = 0;
    while (d) {
        totalRemaining += d->remaining;
        if (d->date > date) ++remCount;
        d = d->next;
    }
    int remainingDaysIncludingToday = 0;
    d = monthlyRecord;
    while (d) { if (d->remaining > 0) remainingDaysIncludingToday++; d = d->next; } // approximate

    // Notify user of new daily limit
    DayRecordNode *fromNow = findDay(monthlyRecord, date);
    int daysLeft = countRemainingDays(fromNow); // includes current day and after
    if (daysLeft > 0) {
        float newDailyLimit = totalRemaining / (float)daysLeft;
        printf("Your recalculated daily spending limit for the rest of month (including today): %.2f\n", newDailyLimit);
    }
}

void displayRemainingBudget(DayRecordNode *monthlyRecord, int date) {
    DayRecordNode *dayRecord = findDay(monthlyRecord, date);
    if (!dayRecord) {
        printf("Invalid date!\n");
    } else {
        printf("Remaining budget for %d: %.2f (daily budget: %.2f)\n", date, dayRecord->remaining, dayRecord->budget);
        // show transactions for that day
        TransactionNode *t = dayRecord->transactions;
        if (!t) printf("  No transactions\n");
        else {
            printf("  Transactions:\n");
            while (t) {
                printf("    %.2f -> %s\n", t->amount, t->category);
                t = t->next;
            }
        }
    }
}

// ---------- Category Summary & Search ----------
void showCategorySummary(DayRecordNode *monthlyRecord, float monthlyBudget) {
    // Simple dynamic list of categories
    typedef struct Cat {
        char name[MAX_CATEGORY_LEN];
        float total;
    } Cat;
    Cat *cats = NULL;
    int ccount = 0;

    DayRecordNode *d = monthlyRecord;
    while (d) {
        TransactionNode *t = d->transactions;
        while (t) {
            int found = -1;
            for (int i = 0; i < ccount; ++i) {
                if (strcmp(cats[i].name, t->category) == 0) { found = i; break; }
            }
            if (found == -1) {
                cats = (Cat *)realloc(cats, sizeof(Cat) * (ccount + 1));
                strncpy(cats[ccount].name, t->category, MAX_CATEGORY_LEN-1);
                cats[ccount].name[MAX_CATEGORY_LEN-1] = '\0';
                cats[ccount].total = t->amount;
                ccount++;
            } else {
                cats[found].total += t->amount;
            }
            t = t->next;
        }
        d = d->next;
    }

    printf("\n--- Category Summary ---\n");
    float totalSpent = 0.0f;
    for (int i = 0; i < ccount; ++i) totalSpent += cats[i].total;
    for (int i = 0; i < ccount; ++i) {
        float pct = (monthlyBudget > 0.0f) ? (cats[i].total / monthlyBudget * 100.0f) : 0.0f;
        printf("%-15s : %.2f (%.2f%% of monthly budget)\n", cats[i].name, cats[i].total, pct);
    }
    printf("Total spent: %.2f\n", totalSpent);
    free(cats);
}

void searchByCategory(DayRecordNode *monthlyRecord, const char *category) {
    printf("\nTransactions for category '%s':\n", category);
    DayRecordNode *d = monthlyRecord;
    int found = 0;
    while (d) {
        TransactionNode *t = d->transactions;
        while (t) {
            if (strcasecmp(t->category, category) == 0) {
                printf("  Date %d -> %.2f\n", d->date, t->amount);
                found = 1;
            }
            t = t->next;
        }
        d = d->next;
    }
    if (!found) printf("  No transactions found for '%s'\n", category);
}

// ---------- Filter / Graph ----------
void filterTransactions(DayRecordNode *monthlyRecord, int start, int end) {
    printf("\nTransactions from %d to %d:\n", start, end);
    DayRecordNode *d = monthlyRecord;
    int found = 0;
    while (d) {
        if (d->date >= start && d->date <= end) {
            TransactionNode *t = d->transactions;
            if (t) {
                printf(" Date %d:\n", d->date);
                while (t) {
                    printf("   %.2f -> %s\n", t->amount, t->category);
                    t = t->next;
                    found = 1;
                }
            }
        }
        d = d->next;
    }
    if (!found) printf("  No transactions in that range.\n");
}

void showGraph(DayRecordNode *monthlyRecord) {
    printf("\n--- Daily Spending Graph (ASCII) ---\n");
    DayRecordNode *d = monthlyRecord;
    while (d) {
        float spent = d->budget - d->remaining;
        if (spent < 0) spent = 0;
        int bars = (int)(spent / 10.0f); // 1 bar per 10 units spent (approx)
        if (bars > 50) bars = 50;
        printf("%2d | ", d->date);
        for (int i = 0; i < bars; ++i) putchar('#');
        printf(" (%.2f)\n", spent);
        d = d->next;
    }
}

// ---------- Edit / Delete ----------
void deleteTransaction(DayRecordNode *monthlyRecord, int date, const char *category) {
    DayRecordNode *d = findDay(monthlyRecord, date);
    if (!d) { printf("Invalid date!\n"); return; }
    TransactionNode *curr = d->transactions, *prev = NULL;
    while (curr && strcasecmp(curr->category, category) != 0) {
        prev = curr;
        curr = curr->next;
    }
    if (!curr) { printf("Transaction not found for that category on date %d.\n", date); return; }
    // refund amount to day's remaining (simple approach)
    d->remaining += curr->amount;
    if (d->remaining > d->budget) d->remaining = d->budget;
    if (!prev) d->transactions = curr->next;
    else prev->next = curr->next;
    free(curr);
    printf("Transaction deleted and amount refunded to the day's remaining.\n");
}

void editTransaction(DayRecordNode *monthlyRecord, int date, const char *category, float newAmount, const char *newCategory) {
    DayRecordNode *d = findDay(monthlyRecord, date);
    if (!d) { printf("Invalid date!\n"); return; }
    TransactionNode *t = d->transactions;
    while (t && strcasecmp(t->category, category) != 0) t = t->next;
    if (!t) { printf("Transaction not found for that category on date %d.\n", date); return; }
    // adjust remaining: refund old then deduct new
    d->remaining += t->amount;
    t->amount = newAmount;
    strncpy(t->category, newCategory, MAX_CATEGORY_LEN-1);
    t->category[MAX_CATEGORY_LEN-1] = '\0';
    d->remaining -= newAmount;
    if (d->remaining < 0) d->remaining = 0;
    printf("Transaction edited.\n");
}

// ---------- Alerts ----------
void checkBudgetAlert(DayRecordNode *record, float monthlyBudget, float thresholdPercent) {
    float totalRemaining = 0.0f;
    DayRecordNode *temp = record;
    while (temp) {
        totalRemaining += temp->remaining;
        temp = temp->next;
    }
    float thresholdVal = monthlyBudget * thresholdPercent / 100.0f;
    if (totalRemaining < thresholdVal) {
        printf("\n⚠ ALERT: Total remaining budget (%.2f) is below %.0f%% (%.2f) of monthly budget!\n", totalRemaining, thresholdPercent, thresholdVal);
    } else {
        printf("\nTotal remaining: %.2f (threshold: %.2f)\n", totalRemaining, thresholdVal);
    }
}

// ---------- Export / Helper ----------
void exportTransactionsCSV(DayRecordNode *monthlyRecord, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        printf("Error opening export file!\n");
        return;
    }
    fprintf(fp, "date,amount,category\n");
    DayRecordNode *d = monthlyRecord;
    while (d) {
        TransactionNode *t = d->transactions;
        while (t) {
            fprintf(fp, "%d,%.2f,%s\n", d->date, t->amount, t->category);
            t = t->next;
        }
        d = d->next;
    }
    fclose(fp);
    printf("Transactions exported to %s\n", filename);
}

// Free all memory
void freeAll(DayRecordNode *monthlyRecord) {
    while (monthlyRecord) {
        DayRecordNode *d = monthlyRecord;
        TransactionNode *t = d->transactions;
        while (t) {
            TransactionNode *tn = t->next;
            free(t);
            t = tn;
        }
        monthlyRecord = monthlyRecord->next;
        free(d);
    }
}

// ---------- Main ----------
int main() {
    float monthlyBudget;
    int daysInMonth;
    int choice;
    int date;
    float amount;
    char category[MAX_CATEGORY_LEN];
    char filenameDays[128] = DAYS_FILENAME;
    char filenameTrans[128] = TRANSACTIONS_FILENAME;
    char password[128];
    strncpy(password, DEFAULT_PASSWORD, sizeof(password)-1);
    password[sizeof(password)-1] = '\0';

    printf("Enter monthly budget: ");
    if (scanf("%f", &monthlyBudget) != 1) return 1;
    printf("Enter number of days in the month: ");
    if (scanf("%d", &daysInMonth) != 1) return 1;
    clearInputBuffer();

    DayRecordNode *monthlyRecord = createMonthRecord(monthlyBudget, daysInMonth);
    float alertThresholdPercent = 20.0f; // default threshold

    do {
        printf("\nMenu:\n");
        printf("1. Add Transaction\n");
        printf("2. View Remaining Budget for a Date\n");
        printf("3. Save Data (password required)\n");
        printf("4. Load Data (password required)\n");
        printf("5. View Category Summary\n");
        printf("6. Filter Transactions by Date Range\n");
        printf("7. Show ASCII Spending Graph\n");
        printf("8. Delete a Transaction\n");
        printf("9. Edit a Transaction\n");
        printf("10. Check Budget Alert (current threshold %.0f%%)\n", alertThresholdPercent);
        printf("11. Export Transactions to CSV\n");
        printf("12. Search Transactions by Category\n");
        printf("13. Archive current files and reset month (new month)\n");
        printf("14. Change Password\n");
        printf("15. Exit\n");
        printf("Enter your choice: ");
        if (scanf("%d", &choice) != 1) { clearInputBuffer(); choice = -1; }
        clearInputBuffer();

        switch (choice) {
            case 1:
                printf("Enter date (1-%d): ", daysInMonth);
                if (scanf("%d", &date) != 1) { clearInputBuffer(); break; }
                printf("Enter amount: ");
                if (scanf("%f", &amount) != 1) { clearInputBuffer(); break; }
                clearInputBuffer();
                printf("Enter category: ");
                if (!fgets(category, sizeof(category), stdin)) { category[0]=0; }
                trim_newline(category);
                addTransaction(monthlyRecord, date, amount, category, daysInMonth, monthlyBudget);
                break;

            case 2:
                printf("Enter date (1-%d): ", daysInMonth);
                if (scanf("%d", &date) != 1) { clearInputBuffer(); break; }
                clearInputBuffer();
                displayRemainingBudget(monthlyRecord, date);
                break;

            case 3:
                if (!passwordCheck(password)) { printf("Wrong password. Save aborted.\n"); break; }
                saveDataToFile(monthlyRecord, filenameDays, filenameTrans, daysInMonth);
                break;

            case 4:
                if (!passwordCheck(password)) { printf("Wrong password. Load aborted.\n"); break; }
                {
                    int loadedDays = 0;
                    DayRecordNode *loaded = loadDaysFromFile(filenameDays, &loadedDays);
                    if (!loaded) {
                        printf("No saved day data found. Load aborted.\n");
                    } else {
                        // free current in-memory and replace
                        freeAll(monthlyRecord);
                        monthlyRecord = loaded;
                        // attach transactions file (if exists)
                        attachTransactionsFromFile(monthlyRecord, filenameTrans);
                        daysInMonth = loadedDays > 0 ? loadedDays : daysInMonth;
                        printf("Data loaded. Days in month set to %d\n", daysInMonth);
                    }
                }
                break;

            case 5:
                showCategorySummary(monthlyRecord, monthlyBudget);
                break;

            case 6:
                {
                    int s,e;
                    printf("Enter start date: "); if (scanf("%d", &s)!=1) { clearInputBuffer(); break; }
                    printf("Enter end date: "); if (scanf("%d", &e)!=1) { clearInputBuffer(); break; }
                    clearInputBuffer();
                    if (s>e) { printf("Start should be <= end.\n"); break; }
                    filterTransactions(monthlyRecord, s, e);
                }
                break;

            case 7:
                showGraph(monthlyRecord);
                break;

            case 8:
                printf("Enter date of transaction to delete: "); if (scanf("%d", &date)!=1) { clearInputBuffer(); break; }
                clearInputBuffer();
                printf("Enter category of transaction to delete: "); if (!fgets(category, sizeof(category), stdin)) { category[0]=0; }
                trim_newline(category);
                deleteTransaction(monthlyRecord, date, category);
                break;

            case 9:
                {
                    char newCat[MAX_CATEGORY_LEN];
                    float newAmt;
                    printf("Enter date of transaction to edit: "); if (scanf("%d", &date)!=1) { clearInputBuffer(); break; }
                    clearInputBuffer();
                    printf("Enter current category of transaction: "); if (!fgets(category, sizeof(category), stdin)) { category[0]=0; }
                    trim_newline(category);
                    printf("Enter new amount: "); if (scanf("%f", &newAmt)!=1) { clearInputBuffer(); break; }
                    clearInputBuffer();
                    printf("Enter new category: "); if (!fgets(newCat, sizeof(newCat), stdin)) { newCat[0]=0; }
                    trim_newline(newCat);
                    editTransaction(monthlyRecord, date, category, newAmt, newCat);
                }
                break;

            case 10:
                printf("Enter alert threshold percent (e.g. 20 for 20%%): ");
                if (scanf("%f", &amount) != 1) { clearInputBuffer(); break; }
                clearInputBuffer();
                alertThresholdPercent = amount;
                checkBudgetAlert(monthlyRecord, monthlyBudget, alertThresholdPercent);
                break;

            case 11:
                exportTransactionsCSV(monthlyRecord, "exported_transactions.csv");
                break;

            case 12:
                printf("Enter category to search: ");
                if (!fgets(category, sizeof(category), stdin)) category[0]=0;
                trim_newline(category);
                searchByCategory(monthlyRecord, category);
                break;

            case 13:
                printf("Are you sure you want to archive current files and reset month? (yes/no): ");
                {
                    char answer[16];
                    if (!fgets(answer, sizeof(answer), stdin)) answer[0]=0;
                    trim_newline(answer);
                    if (strcasecmp(answer, "yes") == 0) {
                        if (!passwordCheck(password)) { printf("Wrong password. Aborted.\n"); break; }
                        archiveCurrentFiles(filenameDays, filenameTrans);
                        freeAll(monthlyRecord);
                        printf("Enter new monthly budget: ");
                        if (scanf("%f", &monthlyBudget) != 1) return 1;
                        printf("Enter number of days in the new month: ");
                        if (scanf("%d", &daysInMonth) != 1) return 1;
                        clearInputBuffer();
                        monthlyRecord = createMonthRecord(monthlyBudget, daysInMonth);
                        printf("New month initialized.\n");
                    } else {
                        printf("Abort reset.\n");
                    }
                }
                break;

            case 14:
                {
                    char oldp[128], newp[128];
                    printf("Enter current password: ");
                    if (scanf("%127s", oldp) != 1) { clearInputBuffer(); break; }
                    clearInputBuffer();
                    if (strcmp(oldp, password) != 0) { printf("Wrong current password.\n"); break; }
                    printf("Enter new password: ");
                    if (scanf("%127s", newp) != 1) { clearInputBuffer(); break; }
                    clearInputBuffer();
                    strncpy(password, newp, sizeof(password)-1); password[sizeof(password)-1]=0;
                    printf("Password changed.\n");
                }
                break;

            case 15:
                printf("Exiting... saving automatically.\n");
                saveDataToFile(monthlyRecord, filenameDays, filenameTrans, daysInMonth);
                break;

            default:
                printf("Invalid choice!\n");
        }

    } while (choice != 15);

    freeAll(monthlyRecord);
    return 0;
}
