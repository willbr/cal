#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

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

struct tm today;

void parse_args(int argc, char* argv[], int* weeks_before, int* weeks_after) {
    *weeks_before = 2;
    *weeks_after = 4;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--weeks-before") == 0 && i + 1 < argc) {
            *weeks_before = atoi(argv[i + 1]);
            i++;
        }
        else if (strcmp(argv[i], "--weeks-after") == 0 && i + 1 < argc) {
            *weeks_after = atoi(argv[i + 1]);
            i++;
        }
    }
}

time_t get_start_of_week(time_t current) {
    struct tm time_info;
    localtime_s(&time_info, &current);
    return current - (time_info.tm_wday * SECONDS_PER_DAY);
}

int is_same_day(struct tm* date) {
    return date->tm_year == today.tm_year &&
           date->tm_mon == today.tm_mon &&
           date->tm_mday == today.tm_mday;
}

void print_calendar_header(char* buffer, size_t size) {
    int pos = 0;
    const char* days[] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};
    
    pos += snprintf(buffer + pos, size - pos, "    ");
    for (int i = 0; i < DAYS_PER_WEEK; i++) {
        if (i == today.tm_wday) {
            pos += snprintf(buffer + pos, size - pos, "%s ", days[i]);
        } else {
            pos += snprintf(buffer + pos, size - pos, DARK_GREY "%s " RESET, days[i]);
        }
    }
    pos += snprintf(buffer + pos, size - pos, "\n");
}

int is_current_month(int month) {
    return month == today.tm_mon;
}

void print_week(time_t start_time, int is_current_week, char* buffer, size_t size) {
    struct tm time_info;
    static const char* month_names[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                      "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    int pos = 0;
    
    // Cache the week's dates
    struct tm days[DAYS_PER_WEEK];
    for (int i = 0; i < DAYS_PER_WEEK; i++) {
        time_t day_time = start_time + (i * SECONDS_PER_DAY);
        localtime_s(&days[i], &day_time);
    }
    
    // Print left month name
    int current_month = days[0].tm_mon;
    if (is_current_week && is_current_month(current_month)) {
        pos += snprintf(buffer + pos, size - pos, "%-4s", month_names[current_month]);
    } else {
        pos += snprintf(buffer + pos, size - pos, DARK_GREY "%-4s" RESET, month_names[current_month]);
    }
    
    // Track month changes while printing days
    int last_month = current_month;
    int month_changed = 0;
    int new_month = -1;
    
    // Print the days
    for (int day = 0; day < DAYS_PER_WEEK; day++) {
        if (days[day].tm_mon != last_month) {
            month_changed = 1;
            new_month = days[day].tm_mon;
        }
        
        if (is_same_day(&days[day])) {
            pos += snprintf(buffer + pos, size - pos, BOLD BG_YELLOW "%2d" RESET " ", 
                          days[day].tm_mday);
        } else {
            pos += snprintf(buffer + pos, size - pos, DARK_GREY "%2d " RESET, 
                          days[day].tm_mday);
        }
        
        last_month = days[day].tm_mon;
    }
    
    // Print the new month name if there was a change
    if (month_changed) {
        if (is_current_week && is_current_month(new_month)) {
            pos += snprintf(buffer + pos, size - pos, "%s", month_names[new_month]);
        } else {
            pos += snprintf(buffer + pos, size - pos, DARK_GREY "%s" RESET, 
                          month_names[new_month]);
        }
    }
    
    pos += snprintf(buffer + pos, size - pos, "\n");
}

int main(int argc, char* argv[]) {
    int weeks_before, weeks_after;
    char buffer[BUFFER_SIZE];
    
    parse_args(argc, argv, &weeks_before, &weeks_after);
    
    if (weeks_before < 0 || weeks_after < 0 || 
        weeks_before > MAX_WEEKS || weeks_after > MAX_WEEKS) {
        printf("Error: Weeks before/after must be between 0 and %d\n", MAX_WEEKS);
        return 1;
    }
    
    time_t current_time = time(NULL);
    struct tm now;
    localtime_s(&now, &current_time);
    today = now;
    
    time_t start_of_current_week = get_start_of_week(current_time);
    
    // Print header
    print_calendar_header(buffer, BUFFER_SIZE);
    printf("%s", buffer);
    
    // Print previous weeks
    for (int i = weeks_before; i > 0; i--) {
        time_t week_time = start_of_current_week - (i * SECONDS_PER_WEEK);
        print_week(week_time, 0, buffer, BUFFER_SIZE);
        printf("%s", buffer);
    }
    
    // Print current week
    print_week(start_of_current_week, 1, buffer, BUFFER_SIZE);
    printf("%s", buffer);
    
    // Print following weeks
    for (int i = 1; i <= weeks_after; i++) {
        time_t week_time = start_of_current_week + (i * SECONDS_PER_WEEK);
        print_week(week_time, 0, buffer, BUFFER_SIZE);
        printf("%s", buffer);
    }
    
    return 0;
}
