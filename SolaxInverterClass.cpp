#include  "SolaxInverterClass.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
SolaxInverter::SolaxInverter( int interval, char  *url ) {
    
  update_interval = interval;
  last_update = millis();
  collect_failures = 0;
  powerq_count = 0;
  strncpy( inverter_url, url, SOLAX_REALTIME_URL_MAX_LEN -1 );
  network_problem_detected = false;
  oled_update_required = false;
  console_logging_enabled = true;
}
  
////////////////////////////////////////////////////////////////////////////////////////////////////
void  SolaxInverter::update() {
  if((millis() - last_update) > update_interval) {
    last_update = millis();
    collectData();                          // Resets collect_failures as soon as a good one is in.
    oled_update_required = true;
    if( collect_failures > 10 ) {
      collect_failures = 5;                // If not reset by collectData(), allow another 5 before it resets wifi again
      network_problem_detected = true;
    } else {
      network_problem_detected = false;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void  SolaxInverter::calculateSMA() {

  float sp_avg = 0, lp_avg = 0, sp_gradient = 0;

  for( int a = POWER_Q_SIZE - 1; a > 0; --a ) {
    powerq[a].solar_power = powerq[a-1].solar_power;
    sp_avg += powerq[a].solar_power;
    powerq[a].solar_power_gradient = powerq[a-1].solar_power_gradient;
    sp_gradient += powerq[a].solar_power_gradient;
    powerq[a].load_power = powerq[a-1].load_power;
    lp_avg += powerq[a].load_power;
  }
  powerq[0].solar_power = solar_power;
  sp_avg += powerq[0].solar_power;
  powerq[0].solar_power_gradient = powerq[0].solar_power - powerq[1].solar_power;
  sp_gradient += powerq[0].solar_power_gradient;
  powerq[0].load_power = load_power;
  lp_avg += powerq[0].load_power;

  if( powerq_count < POWER_Q_SIZE ) {
    powerq_count ++;
  }
  solar_power_sma = sp_avg / powerq_count;
  solar_power_gradient = sp_gradient / powerq_count;
  load_power_sma = lp_avg / powerq_count;  
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void  SolaxInverter::collectData() {
  
  char  buf[1024];
  float current_solvals[MAX_SOLVALS] = {0};
  char  *cp, *cp2, *pp;
  int   c;
  int   http_code;

  if ( console_logging_enabled == true ) {
    Serial.println("CollectSolaxData() ...");
  }

  http.begin(inverter_url);
        
  http_code = http.GET();
  
  if(http_code == HTTP_CODE_OK) {
    strcpy(buf, http.getString().c_str());          // How long is this string ... can it be bound ?
    if( cp = strchr(buf,'[')) {
      cp++;
      if( cp2 = strchr(cp, ']')) {
        *cp2 = '\0';
        c = 0;
        pp = strtok(cp,",");
        do {
          current_solvals[c++] = atof(pp);
        }while(( pp = strtok(NULL, ",")) && (c < MAX_SOLVALS ));
          
        if( c == MAX_SOLVALS ) {
          for( int a = 0; a < MAX_SOLVALS; a++ ) {
            solax_vals[a] = current_solvals[a];
            if ( console_logging_enabled == true ) {
              Serial.printf( " Arg(%02d) %30s = %8.2f\n", a, solax_field_names[a], current_solvals[a] );
            }
          }
          solar_power = solax_vals[PV1_POWER] + solax_vals[PV2_POWER];
          battery_capacity = solax_vals[BATTERY_CAPACITY];
          load_power = solar_power;
          load_power -= solax_vals[FEED_IN_POWER];
          load_power -= solax_vals[BATTERY_POWER];
          calculateSMA();
        } else {
          Serial.printf( "CollectSolaxData - Insufficient arguments c = %d\n", c );
          for( int a = 0; a < c; a++ ) {
            Serial.printf( "Arg(%d) = %8.2f", a, current_solvals[a] );
          }
        }
      }
    }
    if( collect_failures ) {
      Serial.printf( "collectSolaxData - [HTTP] GET success" );
      collect_failures = 0;
    }
  } else {
    collect_failures ++;
    Serial.printf( "collectSolaxData - [HTTP] GET failed ... code: %d - qty(%d)", http_code, collect_failures );
  }
  http.end();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void  SolaxInverter::dumpSolaxData() {
  Serial.println("");
  Serial.println("** Debug dump of solax data **" );
  for( int a = 0; a < MAX_SOLVALS; a++ ) {
    Serial.printf( "Arg(%d) = %8.2f\n", a, solax_vals[a] );
  }  
  Serial.println("******************************" );
  Serial.println("");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void  SolaxInverter::dataTimeStamp() {
    
  Serial.printf("============= SOLAX =============\n");
  Serial.printf("pv1 current          = %8.2fA\n", solax_vals[PV1_CURRENT] );
  Serial.printf("pv2 current          = %8.2fA\n", solax_vals[PV2_CURRENT] );
  Serial.printf("pv1 voltage          = %8.2fV\n", solax_vals[PV1_VOLTAGE] );
  Serial.printf("pv2 voltage          = %8.2fV\n", solax_vals[PV2_VOLTAGE] );
  Serial.printf("grid current         = %8.2fA\n", solax_vals[GRID_CURRENT] );
  Serial.printf("grid voltage         = %8.2fV\n", solax_vals[GRID_VOLTAGE] );
  Serial.printf("grid power           = %8.2fW\n", solax_vals[GRID_POWER] );
  Serial.printf("inner temp           = %8.2f\n", solax_vals[INNER_TEMP] );
  Serial.printf("solar today          = %8.2fkWh\n", solax_vals[SOLAR_TODAY] );
  Serial.printf("solar total          = %8.2fkWh\n", solax_vals[SOLAR_TOTAL] );
  Serial.printf("feed in power        = %8.2fW\n", solax_vals[FEED_IN_POWER] );
  Serial.printf("pv1 power            = %8.2fW\n", solax_vals[PV1_POWER] );
  Serial.printf("pv2 power            = %8.2fW\n", solax_vals[PV2_POWER] );
  Serial.printf("battery voltage      = %8.2fV\n", solax_vals[BATTERY_VOLTAGE] );
  Serial.printf("battery current      = %8.2fA\n", solax_vals[BATTERY_CURRENT] );
  Serial.printf("battery power        = %8.2fW\n", solax_vals[BATTERY_POWER] );
  Serial.printf("battery temp         = %8.2f°C\n", solax_vals[BATTERY_TEMP] );
  Serial.printf("battery capacity     = %8.2f%%\n", solax_vals[BATTERY_CAPACITY] );
  Serial.printf("=========== Calculated ==========\n");  
  Serial.printf("Current Solar Power  = %8.2fW\n", solar_power );      
  Serial.printf("Current Load Power   = %8.2fW\n", load_power );
  Serial.printf("======== Moving Averages ========\n");  
  Serial.printf("Solar Power          = %8.2fW\n", solar_power_sma );
  Serial.printf("Solar Gradient       = %8.2f\n", solar_power_gradient);
  Serial.printf("Load Power           = %8.2fW\n", load_power_sma );
}
