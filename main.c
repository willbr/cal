#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>

// Calendar constants
#define MAX_WEEKS 52

// Time constants
#define SECONDS_PER_MINUTE 60
#define MINUTES_PER_HOUR 60
#define HOURS_PER_DAY 24
#define DAYS_PER_WEEK 7
#define SECONDS_PER_HOUR (SECONDS_PER_MINUTE * MINUTES_PER_HOUR)
#define SECONDS_PER_DAY (SECONDS_PER_HOUR * HOURS_PER_DAY)
#define SECONDS_PER_WEEK (SECONDS_PER_DAY * DAYS_PER_WEEK)

// Buffer size
#define BUFFER_SIZE 512

// ANSI color codes
#define BOLD "\033[1m"
#define RESET "\033[0m"
#define BG_YELLOW "\033[43m"
#define DARK_GREY "\033[90m"

void print_help() {
    printf("Usage: calendar [OPTIONS]\n");
    printf("\nDisplay a calendar centered on the current week with customizable range.\n");
    printf("\nOptions:\n");
    printf("  --help              Display this help message and exit\n");
    printf("  --weeks-before N    Show N weeks before current week (default: 2, max: %d)\n", MAX_WEEKS);
    printf("  --weeks-after N     Show N weeks after current week  (default: 4, max: %d)\n", MAX_WEEKS);
    printf("\nOutput:\n");
    printf("  - Current day is highlighted in yellow\n");
    printf("  - Current week's month is shown in normal text\n");
    printf("  - Other weeks' months are shown in grey\n");
}

int my_localtime(struct tm *result, const time_t *timep) {
#if defined(_WIN32) || defined(_WIN64)
    // localtime_s returns 0 on success.
    return localtime_s(result, timep);
#else
    // localtime_r returns a pointer to result (or NULL on error).
    return (localtime_r(timep, result) == NULL);
#endif
}

void parse_args(int argc, char* argv[], int* weeks_before, int* weeks_after) {
    *weeks_before = 2;
    *weeks_after = 4;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            print_help();
            exit(0);
        }
        else if (strcmp(argv[i], "--weeks-before") == 0 && i + 1 < argc) {
            char* endptr;
            errno = 0;
            long val = strtol(argv[i + 1], &endptr, 10);
            if (errno == 0 && *endptr == '\0') {
                *weeks_before = (int)val;
            }
            i++;
        }
        else if (strcmp(argv[i], "--weeks-after") == 0 && i + 1 < argc) {
            char* endptr;
            errno = 0;
            long val = strtol(argv[i + 1], &endptr, 10);
            if (errno == 0 && *endptr == '\0') {
                *weeks_after = (int)val;
            }
            i++;
        }
        else {
            printf("Unknown option: %s\n", argv[i]);
            printf("Use --help for usage information\n");
            exit(1);
        }
    }
}

//---------------------------------------------------------------------
// Returns the start of the week (Sunday at midnight) for the given time.
// Adjusts the tm struct and uses mktime() for DST-safe normalization.
//---------------------------------------------------------------------
time_t get_start_of_week(time_t current) {
    struct tm time_info;
    if (my_localtime(&time_info, &current) != 0) {
        // Fallback: return current time if conversion fails.
        return current;
    }
    time_info.tm_sec = 0;
    time_info.tm_min = 0;
    time_info.tm_hour = 0;
    // Adjust to previous Sunday (tm_wday is 0 for Sunday).
    time_info.tm_mday -= time_info.tm_wday;
    return mktime(&time_info);
}

int is_same_day(const struct tm* date, const struct tm* today) {
    return date->tm_year == today->tm_year &&
           date->tm_mon == today->tm_mon &&
           date->tm_mday == today->tm_mday;
}

void print_calendar_header(char* buffer, size_t size, const struct tm* today) {
    int pos = 0;
    const char* days[] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};

    pos += snprintf(buffer + pos, size - pos, "    ");
    for (int i = 0; i < DAYS_PER_WEEK; i++) {
        if (i == today->tm_wday) {
            pos += snprintf(buffer + pos, size - pos, "%s ", days[i]);
        } else {
            pos += snprintf(buffer + pos, size - pos, DARK_GREY "%s " RESET, days[i]);
        }
    }
    pos += snprintf(buffer + pos, size - pos, "\n");
}

int is_current_month(int month, const struct tm* today) {
    return month == today->tm_mon;
}

void print_week(time_t start_time, int is_current_week, char* buffer, size_t size, const struct tm* today) {
    struct tm base;
    if (my_localtime(&base, &start_time) != 0) {
        memset(&base, 0, sizeof(base));
    }
    static const char* month_names[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                      "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    int pos = 0;

    // Cache the week's dates using tm arithmetic.
    struct tm days[DAYS_PER_WEEK];
    for (int i = 0; i < DAYS_PER_WEEK; i++) {
        struct tm day_info = base;
        day_info.tm_mday += i;
        time_t day_time = mktime(&day_info); // Normalize time (handles DST)
        if (my_localtime(&days[i], &day_time) != 0) {
            days[i] = day_info; // Fallback if conversion fails.
        }
    }

    int current_month = days[0].tm_mon;
    if (is_current_week && is_current_month(current_month, today)) {
        pos += snprintf(buffer + pos, size - pos, "%-4s", month_names[current_month]);
    } else {
        pos += snprintf(buffer + pos, size - pos, DARK_GREY "%-4s" RESET, month_names[current_month]);
    }

    int last_month = current_month;
    int month_changed = 0;
    int new_month = -1;

    for (int day = 0; day < DAYS_PER_WEEK; day++) {
        if (days[day].tm_mon != last_month) {
            month_changed = 1;
            new_month = days[day].tm_mon;
        }

        if (is_same_day(&days[day], today)) {
            pos += snprintf(buffer + pos, size - pos, BOLD BG_YELLOW "%2d" RESET " ", days[day].tm_mday);
        } else {
            pos += snprintf(buffer + pos, size - pos, DARK_GREY "%2d " RESET, days[day].tm_mday);
        }

        last_month = days[day].tm_mon;
    }

    if (month_changed) {
        if (is_current_week && is_current_month(new_month, today)) {
            pos += snprintf(buffer + pos, size - pos, "%s", month_names[new_month]);
        } else {
            pos += snprintf(buffer + pos, size - pos, DARK_GREY "%s" RESET, month_names[new_month]);
        }
    }

    pos += snprintf(buffer + pos, size - pos, "\n");
}

int main(int argc, char* argv[]) {
    int weeks_before, weeks_after;
    char buffer[BUFFER_SIZE];

    parse_args(argc, argv, &weeks_before, &weeks_after);

    if (weeks_before < 0 || weeks_after < 0 || weeks_before > MAX_WEEKS || weeks_after > MAX_WEEKS) {
        printf("Error: Weeks before/after must be between 0 and %d\n", MAX_WEEKS);
        printf("Use --help for usage information\n");
        return 1;
    }

    time_t current_time = time(NULL);
    struct tm now;
    if (my_localtime(&now, &current_time) != 0) {
        fprintf(stderr, "Error converting current time.\n");
        return 1;
    }
    const struct tm today = now;

    time_t start_of_current_week = get_start_of_week(current_time);

    print_calendar_header(buffer, BUFFER_SIZE, &today);
    printf("%s", buffer);

    // Print previous weeks.
    for (int i = weeks_before; i > 0; i--) {
        time_t week_time = start_of_current_week;
        struct tm week_tm;
        if (my_localtime(&week_tm, &week_time) != 0) {
            // Fallback if conversion fails.
        }
        week_tm.tm_mday -= i * DAYS_PER_WEEK;
        week_time = mktime(&week_tm);
        print_week(week_time, 0, buffer, BUFFER_SIZE, &today);
        printf("%s", buffer);
    }

    // Print current week.
    print_week(start_of_current_week, 1, buffer, BUFFER_SIZE, &today);
    printf("%s", buffer);

    // Print following weeks.
    for (int i = 1; i <= weeks_after; i++) {
        time_t week_time = start_of_current_week;
        struct tm week_tm;
        if (my_localtime(&week_tm, &week_time) != 0) {
            // Fallback if conversion fails.
        }
        week_tm.tm_mday += i * DAYS_PER_WEEK;
        week_time = mktime(&week_tm);
        print_week(week_time, 0, buffer, BUFFER_SIZE, &today);
        printf("%s", buffer);
    }

    return 0;
}
