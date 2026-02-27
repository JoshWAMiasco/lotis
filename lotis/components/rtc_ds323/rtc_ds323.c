/**
 * @file rtc_ds323.c
 * @brief DS3231 Real-Time Clock Driver using ESP-IDF + i2c_helper
 */

#include "rtc_ds323.h"
#include <stdio.h>
#include <string.h>
#include "esp_log.h"

static const char *TAG = "RTC_DS323";

static rtc_config_t rtc_config;
static char datetime_buf[32];

/* BCD Conversion */
static inline uint8_t dec2bcd(uint8_t val) { return ((val/10)<<4) | (val%10); }
static inline uint8_t bcd2dec(uint8_t val) { return ((val>>4)*10) + (val & 0x0F); }

int rtc_ds323_init(const rtc_config_t *config)
{
    if (!config) return RTC_ERR_INVALID;
    memcpy(&rtc_config, config, sizeof(rtc_config));
    // IMPORTANT: do not call i2c_helper_init here to avoid double driver installs.
    // Assume app_main initialized the i2c bus already.
    ESP_LOGI(TAG, "rtc_ds323_init: config stored, using existing I2C port %d", rtc_config.i2c_port);
    return RTC_OK;
}

int rtc_set_time(uint8_t hr, uint8_t min, uint8_t sec)
{
    uint8_t buf[4];
    buf[0] = DS3231_REG_SECONDS;
    buf[1] = dec2bcd(sec);
    buf[2] = dec2bcd(min);
    buf[3] = dec2bcd(hr);

    if (i2c_helper_write(rtc_config.i2c_port, rtc_config.i2c_address, buf, sizeof(buf)) != ESP_OK) {
        return RTC_ERR_I2C;
    }
    return RTC_OK;
}

int rtc_set_date(uint16_t year, uint8_t month, uint8_t day)
{
    uint8_t buf[4];
    buf[0] = DS3231_REG_DATE; // 0x04
    buf[1] = dec2bcd(day);
    buf[2] = dec2bcd(month);
    buf[3] = dec2bcd((uint8_t)(year - 2000));

    if (i2c_helper_write(rtc_config.i2c_port, rtc_config.i2c_address, buf, sizeof(buf)) != ESP_OK) {
        return RTC_ERR_I2C;
    }
    return RTC_OK;
}

int rtc_set_time_format(uint8_t format)
{
    uint8_t reg = DS3231_REG_HOURS;
    uint8_t hr;

    // Read current hours register
    if (i2c_helper_write_read(rtc_config.i2c_port, rtc_config.i2c_address, &reg, 1, &hr, 1) != ESP_OK) {
        return RTC_ERR_I2C;
    }

    // Modify format bit (bit 6: 1=12hr, 0=24hr)
    if (format == RTC_FORMAT_12HR) {
        hr |= 0x40;
    } else {
        hr &= ~0x40;
    }

    // Write back to hours register
    uint8_t buf[2];
    buf[0] = DS3231_REG_HOURS;
    buf[1] = hr;

    if (i2c_helper_write(rtc_config.i2c_port, rtc_config.i2c_address, buf, 2) != ESP_OK) {
        return RTC_ERR_I2C;
    }

    return RTC_OK;
}

int rtc_set_ampm(uint8_t is_pm)
{
    uint8_t reg = DS3231_REG_HOURS;
    uint8_t hr;

    // Read current hours register
    if (i2c_helper_write_read(rtc_config.i2c_port, rtc_config.i2c_address, &reg, 1, &hr, 1) != ESP_OK) {
        return RTC_ERR_I2C;
    }

    // Set 12hr mode (bit 6) and AM/PM (bit 5: 1=PM, 0=AM)
    hr |= 0x40; // Enable 12hr mode
    if (is_pm) {
        hr |= 0x20; // Set PM
    } else {
        hr &= ~0x20; // Set AM
    }

    // Write back to hours register
    uint8_t buf[2];
    buf[0] = DS3231_REG_HOURS;
    buf[1] = hr;

    if (i2c_helper_write(rtc_config.i2c_port, rtc_config.i2c_address, buf, 2) != ESP_OK) {
        return RTC_ERR_I2C;
    }

    return RTC_OK;
}

int rtc_get_time(rtc_time_t *time)
{
    if (!time) return RTC_ERR_INVALID;
    uint8_t reg = DS3231_REG_SECONDS;
    uint8_t data[7];

    if (i2c_helper_write_read(rtc_config.i2c_port, rtc_config.i2c_address, &reg, 1, data, 7) != ESP_OK) {
        return RTC_ERR_I2C;
    }

    time->second = bcd2dec(data[0]);
    time->minute = bcd2dec(data[1]);
    // hours register: bit6 = 12/24 mode, bit5 = AM/PM (when 12hr)
    if (data[2] & 0x40) { // 12-hour
        time->format = RTC_FORMAT_12HR;
        time->is_pm = (data[2] & 0x20) ? RTC_PM : RTC_AM;
        time->hour = bcd2dec(data[2] & 0x1F); // lower 5 bits are hour in 12hr
    } else {
        time->format = RTC_FORMAT_24HR;
        time->is_pm = RTC_AM;
        time->hour = bcd2dec(data[2] & 0x3F);
    }

    time->day = bcd2dec(data[4]);
    time->month = bcd2dec(data[5] & 0x1F);
    time->year = 2000 + bcd2dec(data[6]);

    return RTC_OK;
}

int rtc_get_temp_c(void)
{
    uint8_t reg = DS3231_REG_TEMP_MSB;
    uint8_t buf[2];
    if (i2c_helper_write_read(rtc_config.i2c_port, rtc_config.i2c_address, &reg, 1, buf, 2) != ESP_OK) {
        return RTC_ERR_I2C;
    }
    int8_t msb = (int8_t)buf[0];            // signed integer part
    uint8_t lsb = buf[1];
    int fraction = (lsb >> 6) & 0x03;      // top two bits: 0, .25, .50, .75
    int temp_c = msb;
    if (msb >= 0) {
        // rounding to nearest int
        if (fraction >= 2) temp_c += 1;
    } else {
        // negative temperatures: subtract if fraction != 0
        if (fraction >= 2) temp_c -= 1;
    }
    return temp_c;
}

int rtc_get_temp_f(void)
{
    int c = rtc_get_temp_c();
    if (c < 0) return c;
    return (c * 9 / 5) + 32;
}

int rtc_get_epoch(void)
{
    rtc_time_t time;

    // Read current time from RTC
    if (rtc_get_time(&time) != RTC_OK) {
        return RTC_ERR_I2C;
    }

    // Validate year range (Unix epoch starts at 1970)
    if (time.year < 1970) {
        ESP_LOGE(TAG, "RTC year %d is before Unix epoch (1970)", time.year);
        return RTC_ERR_FAIL;
    }

    // Convert 12-hour format to 24-hour if needed
    uint8_t hour_24 = time.hour;
    if (time.format == RTC_FORMAT_12HR) {
        if (time.hour == 12) {
            hour_24 = (time.is_pm == RTC_PM) ? 12 : 0;
        } else {
            hour_24 = (time.is_pm == RTC_PM) ? (time.hour + 12) : time.hour;
        }
    }

    // Days per month (non-leap year)
    static const uint16_t days_in_month[] = {
        0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };

    // Calculate total days since Unix epoch (Jan 1, 1970)
    uint32_t days = 0;

    // Add days for complete years
    for (uint16_t y = 1970; y < time.year; y++) {
        // Leap year check: divisible by 4, except century years unless divisible by 400
        bool is_leap = (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0));
        days += is_leap ? 366 : 365;
    }

    // Add days for complete months in current year
    for (uint8_t m = 1; m < time.month; m++) {
        days += days_in_month[m];
        // Add leap day if current year is leap and we've passed February
        if (m == 2) {
            bool is_leap = (time.year % 4 == 0 && (time.year % 100 != 0 || time.year % 400 == 0));
            if (is_leap) {
                days += 1;
            }
        }
    }

    // Add days in current month (day is 1-based, subtract 1)
    days += (time.day - 1);

    // Convert to seconds
    uint32_t epoch = days * 86400UL;           // Days to seconds
    epoch += hour_24 * 3600UL;                 // Hours to seconds
    epoch += time.minute * 60UL;               // Minutes to seconds
    epoch += time.second;                      // Seconds

    return (int)epoch;
}

const char* rtc_get_datetime_str(void)
{
    return datetime_buf;
}
