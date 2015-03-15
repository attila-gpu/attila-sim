#ifndef D3DINTERFACE_H_INCLUDED
#define D3DINTERFACE_H_INCLUDED

class IRoot9;
class AIRoot9;

class D3DInterface {
public:
    static void initialize(bool _useACD);
    static void finalize();
    static IRoot9 *get_root_9();
    static AIRoot9 *get_acd_root_9();
    static bool useACD();
private:
    static IRoot9 *i_root_9;
    static AIRoot9 *ai_root_9;
    static bool acd;
    D3DInterface();
};

#endif

