// The menus on lcd used for DDS with an analog control by Juntian Tu at JQI in Jan. 2023
// Inherited and editted from https://github.com/JQIamo/hv-piezo-driver/tree/master/code written by Neal Pisenti at JQI

#ifndef MENUS_H
#define MENUS_H

#include "LCD.h"
#include "LinkedList.h"
#include <string> 
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
    int current_mfreq = 200;
    char unit[4];
    virtual void update();
    virtual void enter();
    virtual void process(char c);
    char place_holder[17];
    virtual void carry();
    unsigned int newFreq;
  private:
    virtual void checker();
    virtual void exit();
    virtual void realtime();
};

class Freq_min: public Freq_max{
  public:
    Freq_min(const char * display_name, LCD * display);
    int current_mfreq = 100;
  private:
    virtual void checker();
    virtual void realtime();
    virtual void exit();
};

extern Root root;
extern Menu analog_setting;
extern Freq_max freq_max;
extern Freq_min freq_min;
extern A_switch analog_switch;
extern Back back;
#endif
