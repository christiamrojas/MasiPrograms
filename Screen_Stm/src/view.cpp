#include "view.h"
//test line
#include "Arduino.h"
#include "global_definitions.h"

float screen_PV[2*PV_MAXPOINTS];
uint16_t screen_PV_i[3+2*PV_MAXPOINTS];
uint8_t screen_PV_n=0;

float screen_pMin,screen_pMax,screen_pGain;
float screen_fMin,screen_fMax,screen_fGain;
float screen_vMin,screen_vMax,screen_vGain;
uint8_t screen_time_decimation;

static float pMin, pMax, vMin, vMax;
static float pOff, pGain;
static float vOff, vGain;

static uint8_t decimation;
static float fValues[3] = {0, 0, 0 };

static int old_3cycles;

static long t;

void view_init()
{
  screen_init();
  enc_init();
  view_pscale(0,50);
  view_fscale(-100,100);
  view_vscale(0,800);
  view_tscale(600);

  pMin=0; 
  pMax=60; 
  vMin=0; 
  vMax=800;
  pGain=180/50;
  vGain=180/800;

  decimation = 1;
  old_3cycles = 0;

  pMin=0;
  pMax=60;
  vMin=0;
  vMax=800;
  pOff=729;
  pGain=179.0/50;
  vOff=288;
  vGain=179.0/800;
  t=millis();
}

void view_flush_rx_buffer(void)
{
  screen_flush_rx_buffer();
}

int view_handler(viewInput *vi, modelInput *mo)
{
  view_check_screen(mo);
  view_check_encoder(mo);
  view_struct_handler(vi);

  return MODEL;
}

void view_get_values_before_reset(uint16_t *values)
{  
  uint16_t screenId = view_get_int_value_from_sram_read(SCREEN_SRAM_ADDR);
  values[0] = screenId;

  float tiVcVal = view_get_float_value_from_sram_read(TI_VC_SRAM_ADDR);
  values[1] = (uint16_t)(tiVcVal * 10);

  uint16_t volVal = view_get_int_value_from_sram_read(VOL_SRAM_ADDR);
  values[2] = volVal;

  uint16_t frVal = view_get_int_value_from_sram_read(FR_SRAM_ADDR);
  values[3] = frVal;

  uint16_t triggerVal = view_get_int_value_from_sram_read(TRIGGER_SRAM_ADDR);
  values[4] = triggerVal;

  uint16_t peepVal = view_get_int_value_from_sram_read(PEEP_SRAM_ADDR);
  values[5] = peepVal;

  uint16_t fio2Val = view_get_int_value_from_sram_read(FIO2_SRAM_ADDR);
  values[6] = fio2Val;

  uint16_t pcVal = view_get_int_value_from_sram_read(PC_SRAM_ADDR);
  values[7] = pcVal;

  float tiVal = view_get_float_value_from_sram_read(TI_SRAM_ADDR);
  values[8] = (uint16_t)(tiVal * 10);

  uint16_t psVal = view_get_int_value_from_sram_read(PS_SRAM_ADDR);
  values[9] = psVal;

  uint16_t cyclingVal = view_get_int_value_from_sram_read(CYCLING_SRAM_ADDR);
  values[10] = cyclingVal;

  uint16_t tapneaVal = view_get_int_value_from_sram_read(TAPNEA_SRAM_ADDR);
  values[11] = tapneaVal;

  uint16_t pmaxAlmVal = view_get_int_value_from_sram_read(ALMPMAX_SRAM_ADDR);
  values[12] = pmaxAlmVal;

  uint16_t pminAlmVal = view_get_int_value_from_sram_read(ALMPMIN_SRAM_ADDR);
  values[13] = pminAlmVal;

  uint16_t vtmaxAlmVal = view_get_int_value_from_sram_read(ALMVTMAX_SRAM_ADDR);
  values[14] = vtmaxAlmVal;

  uint16_t vtminAlmVal = view_get_int_value_from_sram_read(ALMVTMIN_SRAM_ADDR);
  values[15] = vtminAlmVal;

  uint16_t frmaxAlmVal = view_get_int_value_from_sram_read(ALMFRMAX_SRAM_ADDR);
  values[16] = frmaxAlmVal;

  float peepAlmVal = view_get_float_value_from_sram_read(ALMPEEP_SRAM_ADDR);
  values[17] = (uint16_t)(peepAlmVal * 10);

  uint16_t fio2AlmVal = view_get_int_value_from_sram_read(ALMFIO2_SRAM_ADDR);
  values[19] = fio2AlmVal;
}

uint16_t view_get_int_value_from_sram_read(uint16_t addr)
{
  uint8_t dataBuffer[20];
  uint8_t currentScreen = screen_read_sram(addr,dataBuffer, 1);
  
  uint16_t returnVal = (uint16_t)(dataBuffer[3] << 8) | (uint16_t)(dataBuffer[4]);

  return returnVal;
}

float view_get_float_value_from_sram_read(uint16_t addr)
{
  uint8_t dataBuffer[20];
  uint8_t currentScreen = screen_read_sram(addr, dataBuffer, 1);

  uint16_t tempVal = (uint16_t)(dataBuffer[3] << 8) | (uint16_t)(dataBuffer[4]);
  float returnVal = ((float)(tempVal))/10.0;

  return returnVal;
}

void view_check_screen(modelInput *mo)
{
  uint16_t buttonId = view_check_touch();

  if(buttonId)
  {
    if(buttonId != ACK)
    {
      mo->button_data.active = true;
      mo->button_data.buttonId = buttonId;
    }
  }
  else
  {
    mo->button_data.active = false;
  }
}

void view_check_encoder(modelInput* mo)
{
  int e;

  e = enc_read();

  switch(e)
  {
    case 0:
      break;
    case 1:
      mo->encInput_data.active = true;
      mo->encInput_data.buttonStatus = false;
      mo->encInput_data.encValue = 1;
      break;
    case 2:
      mo->encInput_data.active = true;
      mo->encInput_data.buttonStatus = false;
      mo->encInput_data.encValue = 2;
    case 3:
      break;
    case 4:
      break;
    case 5:
      mo->encInput_data.active = true;
      mo->encInput_data.buttonStatus = true;
      mo->encInput_data.encValue = 5;
      break;
    case 6:
      mo->encInput_data.active = true;
      mo->encInput_data.buttonStatus = true;
      mo->encInput_data.encValue = 6;
      break;
    default:
      break;
  }
}

void view_struct_handler(viewInput *vi)
{
  if(vi->ctl_data.active)
  {  
    view_graph_handler(vi->ctl_data.p, vi->ctl_data.f, vi->ctl_data.v, vi->ctl_data.start, vi->ctl_data.pfv_flag);
    vi->ctl_data.active = false;
  }
  
  if(vi->btnCtl_data.active)
  {
    view_button_ctl_handler(vi->btnCtl_data);
    vi->btnCtl_data.active = false;
  }

  if(vi->btnIconCtl_data.active)
  {
    view_button_icon_ctl_handler(vi->btnIconCtl_data.btnId, vi->btnIconCtl_data.btnVal);
    vi->btnIconCtl_data.active = false;
  }

  if(vi->widget_data.active)
  {
    view_widget_handler(vi->widget_data);
    vi->widget_data.active = false;
  }

  if(vi->widgetBulk_data.active)
  {
    view_widget_bulk_handler(vi->widgetBulk_data);
    vi->widgetBulk_data.active = false;
    vi->widgetBulk_data.index = 1;
    memset(vi->widgetBulk_data.widget_array, 0, sizeof(vi->widgetBulk_data.widget_array));
  }
  if(vi->alarm_data.active)
  {
    view_alarm_handler(vi->alarm_data);
    vi->alarm_data.active = false;
  }
  if(vi->buzzer_data.active)
  {
    view_buzzer_handler(vi->buzzer_data);
    vi->buzzer_data.active = false;
  }
  if(vi->screen_data.active)
  {
    view_set_screen(vi->screen_data.screenId);
    vi->screen_data.active = false;
  }
  if(vi->multiBtnCtl_data.active)
  {
    view_multi_button_ctl_handler(vi->multiBtnCtl_data);
    vi->multiBtnCtl_data.active = false;
  }
  if(vi->batteryIcon_data.active)
  {
    view_battery_icon_handler(vi->batteryIcon_data.iconVal);
    vi->batteryIcon_data.active = false;
  }
  if(vi->clearScreen_data.active)
  {
    view_clear_graph_handler(0x0301);
    view_clear_graph_handler(0x0303);
    view_clear_graph_handler(0x0307);
    vi->clearScreen_data.active = false;
  }
  if(vi->ledCtl_data.active)
  {
    enc_set_led_color(vi->ledCtl_data.ledColor);
    vi->ledCtl_data.active = false;
  }
  if(vi->screenRtc_data.active)
  {
    screen_set_rtc(vi->screenRtc_data);
    vi->screenRtc_data.active = false;
  }
}

void view_clear_graph_handler(uint16_t reg)
{
  uint16_t data[1];

  data[0] = 0x0000;
  screen_write_sram(reg, data, 1);
}

void view_battery_icon_handler(uint16_t iconVal)
{
  view_button_icon_ctl_handler(BATTERY_ICON, iconVal);
}

void view_alarm_handler(alarm_struct as){
  screen_write_sram(ALARM_VAL, &as.alarm_id, 1);  
}

void view_buzzer_handler(buzzer_struct bs)
{
  screen_beep_buzzer(bs.buzzTime);
}

void view_widget_bulk_handler(widgetBulk_struct wbd)
{
  for(int i=0; i<wbd.index; i++)
  {
    view_widget_handler(wbd.widget_array[i]);
  }
}

void view_widget_handler(widget_struct ws)
{
  for(int i=0; i<ws.decimal; i++)
  {
    ws.val *= 10;
  }

  // This is done to better compensate for values that should 
  // need a round up, should eventually change to a lookup table 
  // function.
  if(ws.reg == ALMPEEP_VAL)
    ws.val += 0.5;
  uint16_t buttonDispVal = (uint16_t)(ws.val);
  screen_write_sram(ws.reg, &buttonDispVal, 1);
}

void view_graph_handler(float p, float f, float v, bool cycle, bool pfv_flag)
{
  if(pfv_flag){
    view_plot_pfv(cycle, p, f, v);
  }
  view_plot_pv(cycle, p, v);
}

void view_multi_button_ctl_handler(multiBtnCtl_struct multiBtnCtl_data)
{
  for(int i=0; i<multiBtnCtl_data.index; i++)
  {
    screen_write_sram(multiBtnCtl_data.btnArray[i].btnId, &multiBtnCtl_data.btnArray[i].btnVal, 1);
  }
}

void view_button_icon_ctl_handler(uint16_t btnId, uint16_t btnVal)
{
  screen_write_sram(btnId, &btnVal, 1);
}

void view_button_ctl_handler(btnCtl_struct btnCtl_data)
{
  if(btnCtl_data.lastButtonId != 0)
  {
    view_button_icon_ctl_handler(btnCtl_data.lastButtonId, btnCtl_data.lastButtonVal);
  }
  
  view_button_icon_ctl_handler(btnCtl_data.currentButtonId, btnCtl_data.currentButtonVal);
}

uint16_t view_check_touch(void)
{
  uint16_t but;

  but = screen_read_button();
  if(but == 0)
  {
    return 0;
  }
  return but;
}

void view_set_init_screen()
{
  uint8_t data[16] = {0, 7};
  screen_write_reg(3, data, 2);
}

void view_set_screen(uint16_t screenId)
{
  uint16_t data[2];

  data[0] = 0x5A01; //Constant that needs to be defined
  data[1] = screenId;

  screen_write_sram(0x84, data, 2);
}

void view_pscale(float pMin, float pMax)
{
    screen_pMin = pMin;
    screen_pMax = pMax;
    screen_pGain = 4096.0/(pMax - pMin);
}

void view_fscale(float fMin, float fMax)
{
  screen_fMin = fMin;
  screen_fMax = fMax;
  screen_fGain = 4096.0/(fMax - fMin);
}

void view_vscale(float vMin, float vMax)
{
  screen_vMin = vMin;
  screen_vMax = vMax;
  screen_vGain = 4096.0/(vMax - vMin);  
}

void view_tscale(int time_3cycles)
{
  float tmp;

  if(time_3cycles == 0)
  {
    return;
  }

  tmp = old_3cycles;
  tmp /= time_3cycles;

  if((tmp>0.8) && (tmp<1.2))
  {
    return;
  }

  old_3cycles = time_3cycles;
  tmp = time_3cycles;
  tmp /= PFV_PIXWIDTH;
  tmp += 0.5;
  screen_time_decimation = tmp; 
}

void view_set_value(uint16_t reg, float value, uint8_t decimal)
{
  uint16_t int_value;
  
  for(int i=0; i<decimal; i++)
  {
    value *= 10;
  }

  int_value = value;
  screen_write_sram(reg, &int_value, 1);
}

void view_sel_button(uint16_t reg, uint16_t val)
{
  uint16_t valData = val;

  screen_write_sram(reg, &valData, 1);
}

void view_plot_pfv(bool cycle, float cmH2O, float Lmin, float mL)
{
  uint16_t dataBuffer[8];

  fValues[0] += cmH2O;
  fValues[1] += Lmin;
  fValues[2] += mL;

  fValues[0] -= screen_pMin;
  fValues[0] *= screen_pGain;

  fValues[1] -= screen_fMin;
  fValues[1] *= screen_fGain;

  fValues[2] -= screen_vMin;
  fValues[2] *= screen_vGain;

  dataBuffer[0] = 0x5AA5;
  dataBuffer[1] = 0x0300;
  dataBuffer[2] = 0x0001;
  dataBuffer[3] = fValues[0];
  dataBuffer[4] = 0x0101;
  dataBuffer[5] = fValues[1];
  dataBuffer[6] = 0x0301;
  dataBuffer[7] = fValues[2];
  fValues[0] = 0;
  fValues[1] = 0;
  fValues[2] = 0;
  decimation = screen_time_decimation;

  screen_write_sram(0x0310, dataBuffer, 8);
}

void view_plot_pv(bool cycle,float p,float v)
{
  uint16_t circle[6],x,y;
  long t1;

  p*=pGain;
  v*=vGain;
  x=pOff+p;
  y=vOff-v;

  if (cycle)
  {
    screen_PV_i[3+2*screen_PV_n]=screen_PV_i[3];
    screen_PV_i[4+2*screen_PV_n]=screen_PV_i[4];

    screen_PV_i[0]=0x0002;                              // cmd Line
    screen_PV_i[1]=screen_PV_n;                       // Number of beelines
    screen_PV_i[2]=0x07c0;                              // Line Color
    screen_write_sram(LOOP_PV_REG,screen_PV_i,3+2*(screen_PV_n+1));
    screen_PV_n=0;
  }

  t1=millis()-t;
  if ((t1>=0)&&(t1<=100)) return;      // muestra cada 100ms
  t+=100;

  int x1,x2,y1,y2;
  if (screen_PV_n>0)
  {
    x1=screen_PV_i[1+2*screen_PV_n];
    y1=screen_PV_i[2+2*screen_PV_n];
    x2=(x1-x)*(x1-x);
    y2=(y1-y)*(y1-y);
    if ((x2*x2+y2*y2)<20) return;

    circle[0]=0x0005;            // cmd Circle
    circle[1]=1;                 // Number of data packs

    circle[2]=x;//pGain*cmH2O+pOff;      // Center X,Y
    circle[3]=y;//vOff-vGain*mL;
    circle[4]=3;                 // Radius of circle
    circle[5]=0xf800;            // Circle Color
    screen_write_sram(POINT_PV_REG,circle,6);
  }

  if (++screen_PV_n>=PV_MAXPOINTS)
  {
    screen_PV_n=PV_MAXPOINTS;
    return;
  }

  screen_PV_i[1+2*screen_PV_n]=x;
  screen_PV_i[2+2*screen_PV_n]=y;
}

void view_set_screen_reset(void)
{
  uint16_t data[2];

  data[0] = 0x55AA; //Constant that needs to be defined
  data[1] = 0x5AA5;

  screen_write_sram(0x04, data, 2);
  screen_read_ack();
}