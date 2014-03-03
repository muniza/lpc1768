#define TIMEOUT 1000
#define RX_BUFFER 256

Timeout timeout;

char inBuff[RX_BUFFER];
int in = 0;

extern "C" void mbed_reset();

void disable(char which) {
    if (which == 'd') drive.disable();      // Turn outputs off, unattach interrupts
    if (which == 'l')  load.disable();
    running = false;
    pc.printf("Disabled %c\r\n", which);
}
void disableAll() {
    disable('d');
    disable('l');
}
void enable(char which, char mode) {
    if (which == 'd') drive.enable(mode);   // Turn on outputs and PWM generation, attach interrupts, initialize controller
    if (which == 'l')  load.enable(mode);
    running = true;
    pc.printf("Enabled %c %c\r\n", which, mode);

}
void calibrateZero() {
    driveSensors.calibrateZero();
    loadSensors.calibrateZero();
    driveBattery.calibrateZero();
    loadBattery.calibrateZero();
    pc.printf("Zero calibrated for current sensors\r\n");
}
void setPoint(char which, char mode, float val) {
    if (which == 'd') {
        drive.setPoint(val, mode);
    }
    if (which == 'l') {
        load.setPoint(val, mode);
    }
    pc.printf("Set operating point %c %c %f\r\n", which, mode, val);
}

void readSensors() {
    pc.printf("DM: %fV %fA %fW\r\nLM: %fV %fA %fW\r\nMS: %frpm\r\nDB: %fV %fA %fW\r\nLB: %fV %fA %fW\r\n", driveSensors.voltage(), driveSensors.current(), driveSensors.power(), tach.rpm(), loadSensors.voltage(), loadSensors.current(), loadSensors.power(), driveBattery.voltage(), driveBattery.current(), driveBattery.power(), loadBattery.voltage(), loadBattery.current(), loadBattery.power());
}
void remoteReset() {
    mbed_reset();
}
void parse() {
    timeout.detach();
    char cmd[64];
    char arg1 = 0;
    char arg2 = 0;
    float arg3 = 0;
    
    int n = sscanf(inBuff, "%s %c %c %f", cmd, &arg1, &arg2, &arg3);
    
    // If no arguements present
    if (n == 1) {
        if (strstr(cmd,    "disableAll") != NULL) disableAll();
        if (strstr(cmd,   "remoteReset") != NULL) remoteReset();
        if (strstr(cmd, "calibrateZero") != NULL) calibrateZero();
        if (strstr(cmd,   "readSensors") != NULL) readSensors();
    }
    
    if (n == 2) {
        if (strstr(cmd, "disable") != NULL)    disable(arg1);
    }
    if (n == 3) {
        if (strstr(cmd, "enable") != NULL)     enable(arg1, arg2);
    }
    if (n == 4) {
        if (strstr(cmd, "setPoint") != NULL)   setPoint(arg1, arg2, arg3);
    }
    // Kill system if message not received (cable disconnected)
    timeout.attach(&disableAll, TIMEOUT);
}

void serialCheck() {
    while (pc.readable()) {
        if (in >= RX_BUFFER) in = 0;
        inBuff[in] = pc.getc();
        if (inBuff[in] == 127 || inBuff[in] == 8) {
            pc.putc('\r');
            for (int i = 0; i < in; i++) pc.putc(' ');
            pc.putc('\r');
            inBuff[in] = 0;
            if (in > 0) in--;
            inBuff[in] = 0;
            pc.printf("%s", inBuff);
            in--;
        } else pc.putc(inBuff[in]);
        if (inBuff[in] == '\r') {
            inBuff[in] = '\0';
            pc.putc('\n');
            parse();
            in = -1;
        }
        in++;
    }
}


/*
void calibrateZero(char* in, char* out) {
    driveMo.calibrateZero();
    // The other sensors .calibrateZero();
}
RPCFunction rpc_calibrateZero(calibrateZero, "calibrateZero");

void enable(char* in, char* out) {
    if (strcmp(in, "driveMotor") == 0) motor.enable('d');
 //   if (strcmp(in, "loadMotor") == 0) loadMotor.enable('d');
  //  if (strcmp(in, "buckBoost") == 0) converter.enable('d');
}
RPCFunction rpc_enable(&enable, "enable");

void disable(char* in, char* out) {
    if (strcmp(in, "driveMotor") == 0) motor.disable();
 //   if (strcmp(in, "loadMotor") == 0) loadMotor.disable();
  //  if (strcmp(in, "buckBoost") == 0) converter.disable();
}
RPCFunction rpc_disable(&disable, "disable");

void setPoint(char* in, char* out) {
    char obj[64];
    char mode = 0;
    float value = 0;
    sscanf(in, "%s %f %c", obj, &value, &mode);
    if (strcmp(obj, "driveMotor") == 0) motor.setPoint(value, mode);
 // if (strcmp(obj, "loadMotor") == 0) loadMotor.enable(value, mode);
 // if (strcmp(obj, "buckBoost") == 0) converter.enable(value, mode);
}
RPCFunction rpc_setPoint(&setPoint, "setPoint");

void getInputs(char* in, char* out) {
    if (strcmp(in, "driveMotor") == 0) sprintf(out, "V: %f I: %f P: %f S: %f", motor.voltage(), motor.current(), motor.power(), motor.rpm());
 //   if (strcmp(in, "loadMotor") == 0) loadMotor.disable();
  //  if (strcmp(in, "buckBoost") == 0) converter.disable();
}
RPCFunction rpc_setPoint(&setPoint, "setPoint");*/