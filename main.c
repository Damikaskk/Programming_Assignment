#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

#define DB_NAME "atm.db"

typedef struct {
    int id;
    int pin;
    double balance;
    int blocked;
    char ownerName[50];
} Card;

void initializeDatabase();
int fetchCard(int cardId, Card *card);
void updateBalance(int cardId, double newBalance);
void updatePin(int cardId, int newPin);
void blockCard(int cardId);
void contactBank(int cardId);
void handleTransaction(Card *card);
void showMenu();
int withdrawMoney(Card *card);
int depositMoney(Card *card);

int main() {
    initializeDatabase();

    int cardId, enteredPin, attempts;
    Card currentCard;

    while (1) {
        printf("\nEnter Card ID (1 or 2, 0 to Exit): ");
        scanf("%d", &cardId);

        if (cardId == 0) break;
        if (fetchCard(cardId, &currentCard) == 0) {
            printf("Card not found.\n");
            continue;
        }

        if (currentCard.blocked) {
            printf("Card is blocked. Contact the bank.\n");
            contactBank(cardId);
            continue;
        }

        attempts = 0;
        while (attempts < 3) {
            printf("Enter PIN: ");
            scanf("%d", &enteredPin);
            if (enteredPin == currentCard.pin) {
                handleTransaction(&currentCard);
                break;
            }
            printf("Incorrect PIN. Attempts left: %d\n", 2 - attempts);
            attempts++;
        }

        if (attempts == 3) {
            printf("Card blocked. Contact the bank.\n");
            blockCard(cardId);
        }
    }
    return 0;
}

void initializeDatabase() {
    sqlite3 *db;
    char *errMsg = 0;

    if (sqlite3_open(DB_NAME, &db) != SQLITE_OK) {
        printf("Error opening database.\n");
        return;
    }

    const char *sql = "CREATE TABLE IF NOT EXISTS ATM_Cards ("
                      "id INTEGER PRIMARY KEY, "
                      "pin INTEGER, "
                      "balance REAL, "
                      "blocked INTEGER, "
                      "ownerName TEXT);";

    if (sqlite3_exec(db, sql, 0, 0, &errMsg) != SQLITE_OK) {
        printf("SQL Error: %s\n", errMsg);
        sqlite3_free(errMsg);
    }

    sqlite3_close(db);
}

int fetchCard(int cardId, Card *card) {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int found = 0;

    sqlite3_open(DB_NAME, &db);
    char sql[100];
    sprintf(sql, "SELECT * FROM ATM_Cards WHERE id = %d", cardId);

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            card->id = sqlite3_column_int(stmt, 0);
            card->pin = sqlite3_column_int(stmt, 1);
            card->balance = sqlite3_column_double(stmt, 2);
            card->blocked = sqlite3_column_int(stmt, 3);
            strcpy(card->ownerName, (const char *)sqlite3_column_text(stmt, 4));
            found = 1;
        }
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return found;
}

void updateBalance(int cardId, double newBalance) {
    sqlite3 *db;
    char sql[100];

    sqlite3_open(DB_NAME, &db);
    sprintf(sql, "UPDATE ATM_Cards SET balance = %.2f WHERE id = %d", newBalance, cardId);
    sqlite3_exec(db, sql, 0, 0, 0);
    sqlite3_close(db);
}

void updatePin(int cardId, int newPin) {
    sqlite3 *db;
    char sql[100];

    sqlite3_open(DB_NAME, &db);
    sprintf(sql, "UPDATE ATM_Cards SET pin = %d WHERE id = %d", newPin, cardId);
    sqlite3_exec(db, sql, 0, 0, 0);
    sqlite3_close(db);
}

void blockCard(int cardId) {
    sqlite3 *db;
    char sql[100];

    sqlite3_open(DB_NAME, &db);
    sprintf(sql, "UPDATE ATM_Cards SET blocked = 1 WHERE id = %d", cardId);
    sqlite3_exec(db, sql, 0, 0, 0);
    sqlite3_close(db);
}

void contactBank(int cardId) {
    char name[50];
    printf("Enter your full name to unblock the card: ");
    scanf(" %[^\n]s", name);

    sqlite3 *db;
    sqlite3_stmt *stmt;
    int found = 0;

    sqlite3_open(DB_NAME, &db);
    char sql[100];
    sprintf(sql, "SELECT ownerName FROM ATM_Cards WHERE id = %d", cardId);

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *storedName = (const char *)sqlite3_column_text(stmt, 0);
            if (strcmp(storedName, name) == 0) {
                found = 1;
            }
        }
    }
    sqlite3_finalize(stmt);

    if (found) {
        sprintf(sql, "UPDATE ATM_Cards SET blocked = 0 WHERE id = %d", cardId);
        sqlite3_exec(db, sql, 0, 0, 0);
        printf("Card unblocked successfully.\n");
    } else {
        printf("Incorrect name. Card remains blocked.\n");
    }
    sqlite3_close(db);
}

void handleTransaction(Card *card) {
    int option;
    while (1) {
        showMenu();
        scanf("%d", &option);
        switch (option) {
            case 1:
                printf("Your balance: £%.2f\n", card->balance);
                break;
            case 2:
                withdrawMoney(card);
                break;
            case 3:
                depositMoney(card);
                break;
            case 4:
                printf("Enter new PIN: ");
                int newPin;
                scanf("%d", &newPin);
                updatePin(card->id, newPin);
                printf("PIN changed successfully.\n");
                break;
            case 5:
                return;
            default:
                printf("Invalid option.\n");
        }
    }
}

void showMenu() {
    printf("\n1. Check Balance\n");
    printf("2. Withdraw Money\n");
    printf("3. Deposit Money\n");
    printf("4. Change PIN\n");
    printf("5. Exit\n> ");
}

int withdrawMoney(Card *card) {
    double amount;
    printf("Enter amount to withdraw: ");
    scanf("%lf", &amount);
    if (amount > 0 && amount <= card->balance) {
        card->balance -= amount;
        updateBalance(card->id, card->balance);
        printf("Withdrawal successful. New balance: £%.2f\n", card->balance);
    } else {
        printf("Invalid amount.\n");
    }
    return 0;
}

int depositMoney(Card *card) {
    double amount;
    printf("Enter amount to deposit: ");
    scanf("%lf", &amount);
    if (amount > 0) {
        card->balance += amount;
        updateBalance(card->id, card->balance);
        printf("Deposit successful. New balance: £%.2f\n", card->balance);
    }
    return 0;
}
