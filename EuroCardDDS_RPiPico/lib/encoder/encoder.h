// The encoder used for DDS with an analog control by Juntian Tu at JQI in Jan. 2023

#ifndef Encoder_h
#define Encoder_h

class Encoder{
    public:
    int currentStateENC_A = -1;
    int lastStateENC_A = -1;
    int lastreturned = 0; // Used to realize the speed transmission
    int timescounter = 0; // Used to realize the speed transmission
    unsigned long lastButtonPress = 0; // last action timer
    unsigned long lastRotation = 0; // last action timer
    virtual void setup();
    virtual char reader();
};
extern Encoder encoder;
#endif