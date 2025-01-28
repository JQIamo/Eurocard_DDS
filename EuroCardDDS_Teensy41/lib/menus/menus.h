// The menus on lcd used for DDS with an analog control by Juntian Tu at JQI in Jan. 2023
// Inherited and editted from https://github.com/JQIamo/hv-piezo-driver/tree/master/code written by Neal Pisenti at JQI

#ifndef MENUS_H
#define MENUS_H

#include "LCD.h"
#include "LinkedList.h"
#include <string> 
#include "pin_assign.h"
#include "settings.h"

extern uint8_t channel_index;

class Menu {
public:
  Menu(const char * display_name, LCD * display);
  void add(Menu * menu);
  virtual void enter();
  virtual void exit();
  virtual void update();
  virtual void process(char c);
  Menu * _parent;
  Menu * _active;
  char _display_name[17];
  LinkedList<Menu*> _menus;
  uint8_t _current;
  int _menu_counter;
  bool is_executive = false;
protected:
  LCD * _display;
};

class Root: public Menu{
  public:
    Root(const char * display_name, LCD * display);
    // virtual void update();
  private:
    // char current_state[11];
    // virtual void state_translator();
};

class A_switch: public Menu{
  public:
    A_switch(const char * display_name, LCD * display);
    virtual void enter();
    bool is_executive = true;
};

class Back: public Menu{
  public:
    Back(const char * display_name, LCD * display);
    virtual void enter();
    bool is_executive = false;
};

class Freq_max: public Menu{
  public:
    Freq_max(const char * display_name, LCD * display);
    // int current_mfreq = 200;
    char unit[4];
    virtual void update();
    virtual void enter();
    virtual void process(char c);
    char place_holder[17];
    virtual void carry();
    unsigned int newFreq;
    int current_mfreq;
  private:
    virtual void checker();
    virtual void exit();
    virtual void realtime();
};

class Freq_min: public Freq_max{
  public:
    Freq_min(const char * display_name, LCD * display);
  private:
    virtual void checker();
    virtual void realtime();
    virtual void exit();
};

class Channel_set: public Menu{
  public:
    Channel_set(const char * display_name, LCD * display);
    int current_channel;
    virtual void update();
    virtual void process(char c);
    char place_holder[17];
  private:
    virtual void exit();
};

class Static_out: public Menu{
  public:
    Static_out(const char * display_name, LCD * display);
    long current_freq;
    uint8_t cursor;
    char place_holder[17];
    virtual void enter();
    virtual void checker();
    virtual void cursor_update();
    virtual void update();
    virtual void process(char c);
};

class Static_out_amp: public Menu{
  public:
    Static_out_amp(const char * display_name, LCD * display);
    long current_amp;
    char place_holder[17];
    virtual void enter();
    virtual void checker();
    virtual void update();
    virtual void process(char c);
};

extern Root root;
extern Menu analog_setting;
extern Freq_max freq_max;
extern Freq_min freq_min;
extern A_switch analog_switch;
extern Channel_set channel_set;
extern Static_out static_out;
extern Static_out_amp static_out_amp;
extern Back back;
#endif
