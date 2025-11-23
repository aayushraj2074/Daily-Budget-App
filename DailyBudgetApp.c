import csv
import os

# -------------------------------
# Data Structure
# -------------------------------
class DayRecord:
    def __init__(self, date, budget):
        self.date = date
        self.budget = budget
        self.remaining = budget
        self.transactions = []   # list of (amount, category)


# -------------------------------
# Create Month
# -------------------------------
def create_month(monthly_budget, days_in_month):
    per_day = monthly_budget / days_in_month
    month = {}
    for d in range(1, days_in_month + 1):
        month[d] = DayRecord(d, per_day)
    return month


# -------------------------------
# Add Transaction
# -------------------------------
def add_transaction(month, date, amount, category):
    if date not in month:
        print("Invalid date!")
        return
    day = month[date]

    day.transactions.append((amount, category))
    day.remaining -= amount
    if day.remaining < 0:
        day.remaining = 0

    print("Transaction added.")


# -------------------------------
# Show Remaining Budget for a Day
# -------------------------------
def show_remaining(month, date):
    if date not in month:
        print("Invalid date!")
        return
    d = month[date]
    print(f"Day {date} -> Remaining: {d.remaining:.2f} / {d.budget:.2f}")
    if not d.transactions:
        print("  No transactions.")
    else:
        for a, c in d.transactions:
            print(f"  Spent {a} on {c}")


# -------------------------------
# Save Data to CSV
# -------------------------------
def save_data(month, filename="budget_data.csv"):
    with open(filename, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["date", "budget", "remaining", "amount", "category"])

        for d in month.values():
            if not d.transactions:
                # save empty transaction row
                w.writerow([d.date, d.budget, d.remaining, "", ""])
            else:
                for amount, cat in d.transactions:
                    w.writerow([d.date, d.budget, d.remaining, amount, cat])

    print("Data saved.")


# -------------------------------
# Load Data from CSV
# -------------------------------
def load_data(filename="budget_data.csv"):
    if not os.path.exists(filename):
        print("No saved file found.")
        return None, None

    month = {}
    with open(filename, "r") as f:
        r = csv.DictReader(f)
        for row in r:
            date = int(row["date"])
            budget = float(row["budget"])
            remaining = float(row["remaining"])

            if date not in month:
                month[date] = DayRecord(date, budget)
                month[date].remaining = remaining

            if row["amount"]:
                amount = float(row["amount"])
                category = row["category"]
                month[date].transactions.append((amount, category))

    days_in_month = max(month.keys())
    print("Data loaded.")
    return month, days_in_month


# -------------------------------
# Main Menu
# -------------------------------
def main():
    print("=== Monthly Budget Tracker (Simple Version) ===")

    monthly_budget = float(input("Enter monthly budget: "))
    days_in_month = int(input("Enter number of days in month: "))

    month = create_month(monthly_budget, days_in_month)

    while True:
        print("\nMenu:")
        print("1. Add Transaction")
        print("2. View Remaining Budget")
        print("3. Save Data")
        print("4. Load Data")
        print("5. Exit")

        choice = input("Enter choice: ")

        if choice == "1":
            d = int(input("Enter date: "))
            a = float(input("Enter amount: "))
            c = input("Enter category: ")
            add_transaction(month, d, a, c)

        elif choice == "2":
            d = int(input("Enter date: "))
            show_remaining(month, d)

        elif choice == "3":
            save_data(month)

        elif choice == "4":
            month_loaded, days_loaded = load_data()
            if month_loaded:
                month = month_loaded
                days_in_month = days_loaded

        elif choice == "5":
            save_data(month)
            print("Exiting...")
            break

        else:
            print("Invalid choice!")


# Run program
if __name__ == "__main__":
    main()
