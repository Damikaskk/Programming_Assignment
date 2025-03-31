#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int max_attempts() { return 3; }
int card_count() { return 2; }
int pin_max() { return 9999; }
int pin_min() { return 1000; }

typedef struct {
    int pin;
    double balance;
    int blocked;
    char ownerName[50];
} Card;

void showMenu();
void handleTransaction(Card *card);
void printReceipt(const char *operation, double amount, double oldBalance, double newBalance);
int isPinSecure(int pin);
int isSimplePin(int pin);
int isDisallowedPin(int pin);
int isReversedPin(int pin);
void changePin(Card *card);
void contactBank(Card *card);
int askForReceipt();

int withdrawMoney(Card *card);
int depositMoney(Card *card);

int main() {
    // Динамическое выделение памяти для массива карт
    Card *cards = (Card *)malloc(card_count() * sizeof(Card));

    if (cards == NULL) {
        printf("Memory allocation failed!\n");
        return 1;
    }

    // Инициализация карт
    cards[0] = (Card){1234, 1234.60, 0, "Andrew Bradley"};
    cards[1] = (Card){5678, 848.50, 0, "Madiyar Kassim"};

    int choice, enteredPin, attempts;
    while (1) {
        printf("\nSelect your card:\n1. Card 1 (Andrew Bradley)\n2. Card 2 (Madiyar Kassim)\n3. Exit\n> ");
        scanf("%d", &choice);
        if (choice == 3) break;
        if (choice < 1 || choice > card_count()) continue;

        Card *currentCard = &cards[choice - 1];
        if (currentCard->blocked) {
            contactBank(currentCard);
            continue;
        }

        attempts = 0;
        while (attempts < max_attempts()) {
            printf("Enter PIN: ");
            scanf("%d", &enteredPin);
            if (enteredPin == currentCard->pin) {
                handleTransaction(currentCard);
                break;
            }
            printf("Incorrect PIN. Attempts left: %d\n", max_attempts() - (++attempts));
        }
        if (attempts == max_attempts()) {
            printf("Card blocked. Please contact the bank.\n");
            currentCard->blocked = 1;
        }
    }

    // Освобождение памяти
    free(cards);

    return 0;
}

void showMenu() {
    printf("\nSelect an option:\n");
    printf("1. Check Balance\n");
    printf("2. Withdraw Money\n");
    printf("3. Deposit Money\n");
    printf("4. Change PIN\n");
    printf("5. Return Card\n> ");
}

void handleTransaction(Card *card) {
    int option;
    double amount, oldBalance;
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
                changePin(card);
                break;
            case 5:
                printf("Returning card...\n");
                return;
            default:
                printf("Invalid option. Try again.\n");
        }
    }
}

void printReceipt(const char *operation, double amount, double oldBalance, double newBalance) {
    printf("\n=== RECEIPT ===\n");
    printf("Operation: %s\n", operation);
    printf("Amount: £%.2f\n", amount);
    printf("Old Balance: £%.2f\n", oldBalance);
    printf("New Balance: £%.2f\n", newBalance);
    printf("================\n");
}

int withdrawMoney(Card *card) {
    double amount, oldBalance;
    printf("Enter amount to withdraw (multiples of 5, 10, 20): ");
    scanf("%lf", &amount);
    if (amount > 0 && ((int)amount % 5 == 0) && amount <= card->balance) {
        oldBalance = card->balance;
        card->balance -= amount;
        if (askForReceipt()) {
            printReceipt("Withdrawal", amount, oldBalance, card->balance);
        }
    } else {
        printf("Invalid amount or insufficient funds.\n");
    }
    return 0;
}

int depositMoney(Card *card) {
    double amount, oldBalance;
    printf("Enter amount to deposit: ");
    scanf("%lf", &amount);
    if (amount > 0) {
        oldBalance = card->balance;
        card->balance += amount;
        if (askForReceipt()) {
            printReceipt("Deposit", amount, oldBalance, card->balance);
        }
    }
    return 0;
}

int isPinSecure(int pin) {
    return !isSimplePin(pin) && !isReversedPin(pin);
}

int isSimplePin(int pin) {
    int digits[4];
    int tempPin = pin;

    for (int i = 3; i >= 0; --i) {
        digits[i] = tempPin % 10;
        tempPin /= 10;
    }

    if (digits[0] == digits[1] && digits[1] == digits[2] && digits[2] == digits[3]) {
        return 1;
    }

    if ((digits[1] == digits[0] + 1) && (digits[2] == digits[1] + 1) && (digits[3] == digits[2] + 1)) {
        return 1;
    }

    return 0;
}

int isReversedPin(int pin) {
    int digits[4];
    int tempPin = pin;

    for (int i = 3; i >= 0; --i) {
        digits[i] = tempPin % 10;
        tempPin /= 10;
    }

    if ((digits[0] == 9 && digits[1] == 8 && digits[2] == 7 && digits[3] == 6) ||
        (digits[0] == 8 && digits[1] == 7 && digits[2] == 6 && digits[3] == 5) ||
        (digits[0] == 7 && digits[1] == 6 && digits[2] == 5 && digits[3] == 4) ||
        (digits[0] == 6 && digits[1] == 5 && digits[2] == 4 && digits[3] == 3) ||
        (digits[0] == 5 && digits[1] == 4 && digits[2] == 3 && digits[3] == 2) ||
        (digits[0] == 4 && digits[1] == 3 && digits[2] == 2 && digits[3] == 1) ||
        (digits[0] == 3 && digits[1] == 2 && digits[2] == 1 && digits[3] == 0)) {
        return 1;
    }

    return 0;
}

int isDisallowedPin(int pin) {
    return pin == 0;
}

void changePin(Card *card) {
    int newPin;
    while (1) {
        printf("Enter new PIN (4-digit number): ");
        scanf("%d", &newPin);

        if (newPin >= pin_min() && newPin <= pin_max()) {
            if (isPinSecure(newPin) && !isDisallowedPin(newPin)) {
                card->pin = newPin;
                printf("PIN changed successfully.\n");
                break;
            } else if (isDisallowedPin(newPin)) {
                printf("The PIN '0000' is not allowed! Please choose another one.\n");
            } else {
                printf("The PIN is too simple! Please choose a more secure one.\n");
            }
        } else {
            printf("Invalid PIN. Please enter a 4-digit number.\n");
        }
    }
}

void contactBank(Card *card) {
    int choice;
    char name[50];
    printf("Your card is blocked.\n");
    printf("To unblock your card, please select an option:\n");
    printf("1. Enter your full name to unblock\n");
    printf("2. Exit\n> ");
    scanf("%d", &choice);
    getchar();  // Считываем символ новой строки после ввода числа

    if (choice == 1) {
        printf("Enter your full name: ");
        fgets(name, sizeof(name), stdin);  // Используем fgets для считывания строки

        // Убираем символ новой строки в конце строки, если он есть
        name[strcspn(name, "\n")] = '\0';

        if (strcmp(name, card->ownerName) == 0) {
            printf("Your card has been unblocked.\n");
            card->blocked = 0;
        } else {
            printf("Incorrect name. The card cannot be unblocked.\n");
        }
    }
}

int askForReceipt() {
    char choice;
    printf("Do you want a receipt? (Y/N): ");
    scanf(" %c", &choice);
    return (choice == 'Y' || choice == 'y');
}