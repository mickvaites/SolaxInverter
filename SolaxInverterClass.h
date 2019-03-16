#ifndef SolaxInverterClass_h
#define SolaxInverterClass_h

////////////////////////////////////////////////////////////////////////////////////////////////////

#include  <HTTPClient.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// SolaxInverterClasses.h
//
// Classes used to make connections to Solax and Power Switches
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
#define SOLAX_REALTIME_URL_MAX_LEN    100

#define POWER_Q_SIZE      120
#define MAX_SOLVALS       18

#define PV1_CURRENT       0
#define PV2_CURRENT       1
#define PV1_VOLTAGE       2
#define PV2_VOLTAGE       3
#define GRID_CURRENT      4
#define GRID_VOLTAGE      5
#define GRID_POWER        6
#define INNER_TEMP        7
#define SOLAR_TODAY       8
#define SOLAR_TOTAL       9
#define FEED_IN_POWER     10
#define PV1_POWER         11
#define PV2_POWER         12
#define BATTERY_VOLTAGE   13
#define BATTERY_CURRENT   14
#define BATTERY_POWER     15
#define BATTERY_TEMP      16
#define BATTERY_CAPACITY  17

////////////////////////////////////////////////////////////////////////////////////////////////////

class SolaxInverter {

private:

  HTTPClient    http;
  
  int           update_interval;
  unsigned long last_update;
  int           collect_failures;

  char          inverter_url[SOLAX_REALTIME_URL_MAX_LEN] = {0};


  struct  _powerq {
    float     load_power;
    float     solar_power;
    float     solar_power_gradient;
  }powerq[POWER_Q_SIZE];

  const char  *solax_field_names[MAX_SOLVALS + 1] = { "PV1 Current", "PV2 Current", "PV1 Voltage", "PV2 Voltage",
                                                  "Grid Current", "Grid Voltage", "Grid Power",
                                                  "Inner Temporature", "Solar Today", "Solar Total", "Feed in Power",
                                                  "PV1 Power", "PV2 Power",
                                                  "Battery Voltage", "Battery Current", "Battery Power", "Battery Temporature", "Battery Capacity", 0 };

  void  calculateSMA();
  void  collectData();

public:

  float   solax_vals[MAX_SOLVALS] = {0};
  float   solar_power = 0;
  float   solar_power_sma = 0;
  float   solar_power_gradient = 0;
  float   load_power = 0;
  float   load_power_sma = 0;
  float   battery_capacity = 0;
  unsigned int  powerq_count;

  bool    oled_update_required;
  bool    network_problem_detected;
  bool    console_logging_enabled;
   

  SolaxInverter( int, char * );    
  void  update();
  void  dumpSolaxData();
  void  dataTimeStamp();
};

#endif
