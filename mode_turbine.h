/* Turbine Spiro header file */
#define TURBINE_MODE 0x32
#define TURBINE_INIT &initTurbine
#define TURBINE_RUN &runTurbine
#define TURBINE_STOP &stopTurbine
#define TURBINE_TA3_CC_INT &ta3ccTurbine
#define TURBINE_SAVE &save_file

void turbine_setupTA3(void);
void turbine_startTA3(void);

void initTurbine(void);
void runTurbine(void);
void stopTurbine(void);
void ta3ccTurbine(void);

void write_my_charfile(char* filen, unsigned char* data2, int num);
void read_my_charfile(char* filen,unsigned char* data1, int num);
void save_file(void);
void index_file(void);
