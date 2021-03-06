#include <cstdio>
#include <libs/date/includes/date/date.h>
#include <Components/DateTime/DateTimeController.h>
#include <libs/lvgl/lvgl.h>
#include "Clock.h"
#include "../DisplayApp.h"
#include "BatteryIcon.h"
#include "BleIcon.h"
#include "Symbols.h"
using namespace Pinetime::Applications::Screens;
extern lv_font_t jetbrains_mono_extrabold_compressed;
extern lv_font_t jetbrains_mono_bold_20;
extern lv_style_t* LabelBigStyle;

static lv_obj_t* dayofweekname;
static lv_obj_t* ampmvalue;

static void event_handler(lv_obj_t * obj, lv_event_t event) {
  Clock* screen = static_cast<Clock *>(obj->user_data);
  screen->OnObjectEvent(obj, event);
}

Clock::Clock(DisplayApp* app,
        Controllers::DateTime& dateTimeController,
        Controllers::Battery& batteryController,
        Controllers::Ble& bleController) : Screen(app), currentDateTime{{}},
                                           dateTimeController{dateTimeController}, batteryController{batteryController}, bleController{bleController} {
  displayedChar[0] = 0;
  displayedChar[1] = 0;
  displayedChar[2] = 0;
  displayedChar[3] = 0;
  displayedChar[4] = 0;

  bleIcon = lv_label_create(lv_scr_act(), NULL);
  lv_label_set_text(bleIcon, Symbols::bluetooth);
  lv_obj_align(bleIcon, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, 0);
                                             
  batteryIcon = lv_label_create(lv_scr_act(), NULL);
  lv_label_set_text(batteryIcon, "---%");
  lv_obj_align(batteryIcon, bleIcon, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
                                             
  batteryPlug = lv_label_create(lv_scr_act(), NULL);
  lv_label_set_text(batteryPlug, Symbols::plug);
  lv_obj_align(batteryPlug, batteryIcon, LV_ALIGN_OUT_RIGHT_MID, 0, 0);                                          
                                           
  label_date = lv_label_create(lv_scr_act(), NULL);

  lv_obj_align(label_date, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, 0, 0);
                                             
  dayofweekname = lv_label_create(lv_scr_act(), NULL);
                                             
  lv_obj_align(dayofweekname, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 0);
                                             
  ampmvalue = lv_label_create(lv_scr_act(), NULL);
                                             
  lv_obj_align(ampmvalue, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, 0, -20);                                           

  label_time = lv_label_create(lv_scr_act(), NULL);
  lv_label_set_style(label_time, LV_LABEL_STYLE_MAIN, LabelBigStyle);
  lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, 0, -60);

  backgroundLabel = lv_label_create(lv_scr_act(), NULL);
  backgroundLabel->user_data = this;
  lv_obj_set_click(backgroundLabel, true);
  lv_obj_set_event_cb(backgroundLabel, event_handler);
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CROP);
  lv_obj_set_size(backgroundLabel, 240, 240);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text(backgroundLabel, "");

  /*
  
  heartbeatIcon = lv_label_create(lv_scr_act(), NULL);
  lv_label_set_text(heartbeatIcon, Symbols::heartBeat);
  lv_obj_align(heartbeatIcon, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 5, -2);

  heartbeatValue = lv_label_create(lv_scr_act(), NULL);
  lv_label_set_text(heartbeatValue, "0");
  lv_obj_align(heartbeatValue, heartbeatIcon, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

  heartbeatBpm = lv_label_create(lv_scr_act(), NULL);
  lv_label_set_text(heartbeatBpm, "BPM");
  lv_obj_align(heartbeatBpm, heartbeatValue, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

  stepValue = lv_label_create(lv_scr_act(), NULL);
  lv_label_set_text(stepValue, "0");
  lv_obj_align(stepValue, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, -5, -2);

  stepIcon = lv_label_create(lv_scr_act(), NULL);
  lv_label_set_text(stepIcon, Symbols::shoe);
  lv_obj_align(stepIcon, stepValue, LV_ALIGN_OUT_LEFT_MID, -5, 0);
  
  */
}

Clock::~Clock() {
  lv_obj_clean(lv_scr_act());
}

bool Clock::Refresh() {
  batteryPercentRemaining = batteryController.PercentRemaining();
  if (batteryPercentRemaining.IsUpdated()) {
    auto batteryPercent = batteryPercentRemaining.Get();
    char bozo[5];
    sprintf(bozo, "%lu%%", batteryPercent);
    lv_label_set_text(batteryIcon, bozo);
    lv_obj_align(batteryIcon, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, -5, 2);
    auto isCharging = batteryController.IsCharging() || batteryController.IsPowerPresent();
    lv_label_set_text(batteryPlug, BatteryIcon::GetPlugIcon(isCharging));
  }

  bleState = bleController.IsConnected();
  if (bleState.IsUpdated()) {
    if(bleState.Get() == true) {
      lv_label_set_text(bleIcon, BleIcon::GetIcon(true));
    } else {
      lv_label_set_text(bleIcon, BleIcon::GetIcon(false));
    }
  }
  lv_obj_align(bleIcon, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, 0);
  lv_obj_align(batteryIcon, bleIcon, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
  lv_obj_align(batteryPlug, batteryIcon, LV_ALIGN_OUT_RIGHT_MID, 0, 0); 

  currentDateTime = dateTimeController.CurrentDateTime();

  if(currentDateTime.IsUpdated()) {
    auto newDateTime = currentDateTime.Get();

    auto dp = date::floor<date::days>(newDateTime);
    auto time = date::make_time(newDateTime-dp);
    auto yearMonthDay = date::year_month_day(dp);

    auto year = (int)yearMonthDay.year();
    auto month = static_cast<Pinetime::Controllers::DateTime::Months>((unsigned)yearMonthDay.month());
    auto day = (unsigned)yearMonthDay.day();
    auto dayOfWeek = static_cast<Pinetime::Controllers::DateTime::Days>(date::weekday(yearMonthDay).iso_encoding());
    
    char ampm[3];
    
    auto hour = time.hours().count();
    // Convert 24 hour clock to 12 hour
    sprintf(ampm, "%s", "AM");
    if (hour >= 12) {
      sprintf(ampm, "%s", "PM");
      hour = hour - 12;
    }
    if (hour == 0) hour = 12;
    lv_label_set_text(ampmvalue, ampm);
    lv_obj_align(ampmvalue, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, 0, -20);     
    
    auto minute = time.minutes().count();
    auto second = time.seconds().count();

    char minutesChar[3];
    sprintf(minutesChar, "%02d", minute);

    char hoursChar[3];
    sprintf(hoursChar, "%02d", hour);

    char timeStr[6];
    char highorderdigit;
    if (hour > 9) highorderdigit = hoursChar[0];
    else highorderdigit = ' ';
    sprintf(timeStr, "%c%c:%c%c", highorderdigit,hoursChar[1],minutesChar[0], minutesChar[1]);

    if(hoursChar[0] != displayedChar[0] || hoursChar[1] != displayedChar[1] || minutesChar[0] != displayedChar[2] || minutesChar[1] != displayedChar[3]) {
      displayedChar[0] = hoursChar[0];
      displayedChar[1] = hoursChar[1];
      displayedChar[2] = minutesChar[0];
      displayedChar[3] = minutesChar[1];

      lv_label_set_text(label_time, timeStr);
      lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, 0, -60);
      
    }

    if ((year != currentYear) || (month != currentMonth) || (dayOfWeek != currentDayOfWeek) || (day != currentDay)) {
      char dateStr[22];
      sprintf(dateStr, "%s %d", MonthToString(month), day);
      lv_label_set_text(label_date, dateStr);
      lv_obj_align(label_date, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, 0, 0);
      char weekdayStr[9];
      sprintf(weekdayStr, "%s", DayOfWeekToString(dayOfWeek));
      lv_label_set_text(dayofweekname, weekdayStr);
      lv_obj_align(dayofweekname, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 0); 

      currentYear = year;
      currentMonth = month;
      currentDayOfWeek = dayOfWeek;
      currentDay = day;
    }
  }

  /*  
  
  // TODO heartbeat = heartBeatController.GetValue();
  if(heartbeat.IsUpdated()) {
    char heartbeatBuffer[4];
    sprintf(heartbeatBuffer, "%d", heartbeat.Get());
    lv_label_set_text(heartbeatValue, heartbeatBuffer);
    lv_obj_align(heartbeatIcon, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 5, -2);
    lv_obj_align(heartbeatValue, heartbeatIcon, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    lv_obj_align(heartbeatBpm, heartbeatValue, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
  }

  // TODO stepCount = stepController.GetValue();
  if(stepCount.IsUpdated()) {
    char stepBuffer[5];
    sprintf(stepBuffer, "%lu", stepCount.Get());
    lv_label_set_text(stepValue, stepBuffer);
    lv_obj_align(stepValue, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, -5, -2);
    lv_obj_align(stepIcon, stepValue, LV_ALIGN_OUT_LEFT_MID, -5, 0);
  }
  
  */

  return running;
}

const char *Clock::MonthToString(Pinetime::Controllers::DateTime::Months month) {
  return Clock::MonthsString[static_cast<uint8_t>(month)];
}

const char *Clock::DayOfWeekToString(Pinetime::Controllers::DateTime::Days dayOfWeek) {
  return Clock::DaysString[static_cast<uint8_t>(dayOfWeek)];
}

char const *Clock::DaysString[] = {
        "",
        "MONDAY",
        "TUESDAY",
        "WEDNESDAY",
        "THURSDAY",
        "FRIDAY",
        "SATURDAY",
        "SUNDAY"
};

char const *Clock::MonthsString[] = {
        "",
        "January",
        "February",
        "March",
        "April",
        "May",
        "June",
        "July",
        "August",
        "September",
        "October",
        "November",
        "December"
};

void Clock::OnObjectEvent(lv_obj_t *obj, lv_event_t event) {
  if(obj == backgroundLabel) {
    if (event == LV_EVENT_CLICKED) {

      running = false;
    }
  }
}

bool Clock::OnButtonPushed() {
  running = false;
  return false;
}


