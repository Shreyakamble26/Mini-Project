
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_BOOKINGS 500

typedef struct
{
    int id;
    int room;
    char name[50];
    char phone[20];
    char checkin[11];
    char checkout[11];
    double total;
    int active;
} Booking;

typedef struct
{
    int number;
    char type[20];
    double rate;
} Room;

Room rooms[] = {
    {101, "Standard", 1500},
    {102, "Standard", 1500},
    {103, "Standard", 1500},
    {104, "Standard", 1500},
    {105, "Standard", 1500},
    {201, "Deluxe", 2500},
    {202, "Deluxe", 2500},
    {203, "Deluxe", 2500},
    {301, "Suite", 4000},
    {302, "Suite", 4000}
};

int room_count = 10;

void read_line(char text[], int size)
{
    if (fgets(text, size, stdin) == NULL)
    {
        text[0] = '\0';
        return;
    }

    text[strcspn(text, "\n")] = '\0';

    for (int i = 0; text[i] != '\0'; i++)
    {
        if (text[i] == '|')
            text[i] = '-';
    }
}

int read_integer()
{
    char input[50];
    char *end;
    long value;

    while (1)
    {
        read_line(input, sizeof(input));
        value = strtol(input, &end, 10);

        while (*end == ' ' || *end == '\t')
            end++;

        if (end != input && (*end == '\0'))
            return (int)value;

        printf("Enter a valid number: ");
    }
}

int leap_year(int year)
{
    return (year % 400 == 0 ||
           (year % 4 == 0 && year % 100 != 0));
}

long date_to_days(int year, int month, int day)
{
    year -= month <= 2;

    int era = (year >= 0 ? year : year - 399) / 400;
    unsigned yoe = year - era * 400;

    int shifted_month = month + (month > 2 ? -3 : 9);

    unsigned doy = (153 * shifted_month + 2) / 5 + day - 1;
    unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;

    return era * 146097L + doe;
}

int valid_date(char date[], long *serial)
{
    int year, month, day;
    int days_in_month[] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
    };

    if (strlen(date) != 10 ||
        date[4] != '-' || date[7] != '-')
        return 0;

    for (int i = 0; i < 10; i++)
    {
        if (i != 4 && i != 7 && !isdigit((unsigned char)date[i]))
            return 0;
    }

    if (sscanf(date, "%4d-%2d-%2d", &year, &month, &day) != 3)
        return 0;

    if (year < 2020 || month < 1 || month > 12)
        return 0;

    if (leap_year(year))
        days_in_month[1] = 29;

    if (day < 1 || day > days_in_month[month - 1])
        return 0;

    *serial = date_to_days(year, month, day);

    return 1;
}

void read_date(char date[], long *serial)
{
    while (1)
    {
        read_line(date, 11);

        if (valid_date(date, serial))
            return;

        printf("Invalid date! Enter in YYYY-MM-DD format: ");
    }
}

void load_bookings(Booking bookings[], int *count)
{
    FILE *fp = fopen("bookings.txt", "r");

    *count = 0;

    if (fp == NULL)
        return;

    while (*count < MAX_BOOKINGS &&
           fscanf(fp, "%d|%d|%49[^|]|%19[^|]|%10[^|]|%10[^|]|%lf|%d\n",
                  &bookings[*count].id,
                  &bookings[*count].room,
                  bookings[*count].name,
                  bookings[*count].phone,
                  bookings[*count].checkin,
                  bookings[*count].checkout,
                  &bookings[*count].total,
                  &bookings[*count].active) == 8)
    {
        (*count)++;
    }

    fclose(fp);
}

void save_bookings(Booking bookings[], int count)
{
    FILE *fp = fopen("bookings.txt", "w");

    if (fp == NULL)
    {
        printf("Error saving booking records!\n");
        return;
    }

    for (int i = 0; i < count; i++)
    {
        fprintf(fp, "%d|%d|%s|%s|%s|%s|%.2f|%d\n",
                bookings[i].id,
                bookings[i].room,
                bookings[i].name,
                bookings[i].phone,
                bookings[i].checkin,
                bookings[i].checkout,
                bookings[i].total,
                bookings[i].active);
    }

    fclose(fp);
}

int find_room(int number)
{
    for (int i = 0; i < room_count; i++)
    {
        if (rooms[i].number == number)
            return i;
    }

    return -1;
}

int available(int room, long start, long end,
              Booking bookings[], int count)
{
    for (int i = 0; i < count; i++)
    {
        if (bookings[i].active &&
            bookings[i].room == room)
        {
            long old_start, old_end;

            if (valid_date(bookings[i].checkin, &old_start) &&
                valid_date(bookings[i].checkout, &old_end))
            {
                if (start < old_end && old_start < end)
                    return 0;
            }
        }
    }

    return 1;
}

void view_rooms()
{
    printf("\n========== HOTEL ROOM DETAILS ==========\n");

    printf("%-12s %-15s %-15s\n",
           "Room Number", "Room Type", "Price/Night");

    for (int i = 0; i < room_count; i++)
    {
        printf("%-12d %-15s Rs. %-10.2f\n",
               rooms[i].number,
               rooms[i].type,
               rooms[i].rate);
    }
}

int get_dates(char checkin[], char checkout[],
              long *start, long *end)
{
    printf("Enter check-in date (YYYY-MM-DD): ");
    read_date(checkin, start);

    printf("Enter check-out date (YYYY-MM-DD): ");
    read_date(checkout, end);

    if (*end <= *start)
    {
        printf("Check-out must be after check-in!\n");
        return 0;
    }

    return 1;
}

void check_availability(Booking bookings[], int count)
{
    char checkin[11], checkout[11];
    long start, end;

    printf("\n========== CHECK AVAILABILITY ==========\n");

    if (!get_dates(checkin, checkout, &start, &end))
        return;

    int found = 0;

    printf("\nAvailable rooms:\n");

    for (int i = 0; i < room_count; i++)
    {
        if (available(rooms[i].number, start, end,
                      bookings, count))
        {
            printf("Room %d | %s | Rs. %.2f/night\n",
                   rooms[i].number,
                   rooms[i].type,
                   rooms[i].rate);

            found = 1;
        }
    }

    if (!found)
        printf("No rooms available for these dates.\n");
}

int next_booking_id(Booking bookings[], int count)
{
    int max_id = 1000;

    for (int i = 0; i < count; i++)
    {
        if (bookings[i].id > max_id)
            max_id = bookings[i].id;
    }

    return max_id + 1;
}

void book_room(Booking bookings[], int *count)
{
    if (*count >= MAX_BOOKINGS)
    {
        printf("Booking storage is full!\n");
        return;
    }

    Booking b;
    long start, end;

    printf("\n========== NEW ROOM BOOKING ==========\n");

    if (!get_dates(b.checkin, b.checkout, &start, &end))
        return;

    printf("\nAvailable rooms:\n");

    int found = 0;

    for (int i = 0; i < room_count; i++)
    {
        if (available(rooms[i].number, start, end,
                      bookings, *count))
        {
            printf("Room %d | %s | Rs. %.2f/night\n",
                   rooms[i].number,
                   rooms[i].type,
                   rooms[i].rate);

            found = 1;
        }
    }

    if (!found)
    {
        printf("Sorry! No rooms available.\n");
        return;
    }

    printf("\nEnter room number: ");
    int number = read_integer();

    int index = find_room(number);

    if (index == -1 ||
        !available(number, start, end, bookings, *count))
    {
        printf("Invalid or unavailable room!\n");
        return;
    }

    b.room = number;

    printf("Enter customer name: ");
    read_line(b.name, sizeof(b.name));

    printf("Enter phone number: ");
    read_line(b.phone, sizeof(b.phone));

    if (strlen(b.name) == 0 || strlen(b.phone) == 0)
    {
        printf("Customer details cannot be empty!\n");
        return;
    }

    b.id = next_booking_id(bookings, *count);

    long nights = end - start;
    b.total = nights * rooms[index].rate;
    b.active = 1;

    bookings[*count] = b;
    (*count)++;

    save_bookings(bookings, *count);

    printf("\n========== BOOKING CONFIRMED ==========\n");
    printf("Booking ID: %d\n", b.id);
    printf("Customer: %s\n", b.name);
    printf("Room Number: %d\n", b.room);
    printf("Room Type: %s\n", rooms[index].type);
    printf("Check-in: %s\n", b.checkin);
    printf("Check-out: %s\n", b.checkout);
    printf("Total Nights: %ld\n", nights);
    printf("Total Bill: Rs. %.2f\n", b.total);
}

void view_bookings(Booking bookings[], int count)
{
    int found = 0;

    printf("\n========== ACTIVE BOOKINGS ==========\n");

    for (int i = 0; i < count; i++)
    {
        if (bookings[i].active)
        {
            printf("\nBooking ID: %d\n", bookings[i].id);
            printf("Customer: %s\n", bookings[i].name);
            printf("Phone: %s\n", bookings[i].phone);
            printf("Room: %d\n", bookings[i].room);
            printf("Check-in: %s\n", bookings[i].checkin);
            printf("Check-out: %s\n", bookings[i].checkout);
            printf("Bill: Rs. %.2f\n", bookings[i].total);

            found = 1;
        }
    }

    if (!found)
        printf("No active bookings found!\n");
}

int search_booking(Booking bookings[], int count, int id)
{
    for (int i = 0; i < count; i++)
    {
        if (bookings[i].id == id && bookings[i].active)
            return i;
    }

    return -1;
}

void search_record(Booking bookings[], int count)
{
    int id;

    printf("\nEnter booking ID: ");
    id = read_integer();

    int index = search_booking(bookings, count, id);

    if (index == -1)
    {
        printf("Booking not found!\n");
        return;
    }

    printf("\n========== BOOKING DETAILS ==========\n");
    printf("Booking ID: %d\n", bookings[index].id);
    printf("Customer: %s\n", bookings[index].name);
    printf("Phone: %s\n", bookings[index].phone);
    printf("Room: %d\n", bookings[index].room);
    printf("Check-in: %s\n", bookings[index].checkin);
    printf("Check-out: %s\n", bookings[index].checkout);
    printf("Total Bill: Rs. %.2f\n", bookings[index].total);
}

void cancel_booking(Booking bookings[], int count)
{
    int id;

    printf("\nEnter booking ID to cancel: ");
    id = read_integer();

    int index = search_booking(bookings, count, id);

    if (index == -1)
    {
        printf("Active booking not found!\n");
        return;
    }

    bookings[index].active = 0;

    save_bookings(bookings, count);

    printf("Booking cancelled successfully!\n");
}

int main()
{
    Booking bookings[MAX_BOOKINGS];
    int count = 0;
    int choice;

    load_bookings(bookings, &count);

    printf("\n========================================\n");
    printf("       HOTEL ROOM BOOKING SYSTEM\n");
    printf("========================================\n");
    printf("Welcome to our Hotel Management System!\n");

    do
    {
        printf("\n============== MAIN MENU ==============\n");
        printf("1. View Room Details\n");
        printf("2. Check Room Availability\n");
        printf("3. Book a Room\n");
        printf("4. View All Bookings\n");
        printf("5. Search Booking\n");
        printf("6. Cancel Booking\n");
        printf("7. Exit\n");

        printf("\nEnter your choice: ");
        choice = read_integer();

        switch (choice)
        {
            case 1:
                view_rooms();
                break;

            case 2:
                check_availability(bookings, count);
                break;

            case 3:
                book_room(bookings, &count);
                break;

            case 4:
                view_bookings(bookings, count);
                break;

            case 5:
                search_record(bookings, count);
                break;

            case 6:
                cancel_booking(bookings, count);
                break;

            case 7:
                printf("\nThank you for using Hotel Room Booking System!\n");
                printf("All booking records are saved.\n");
                break;

            default:
                printf("\nInvalid choice! Please try again.\n");
        }

    } while (choice != 7);

    return 0;
}
