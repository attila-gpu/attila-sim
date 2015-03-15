#ifndef TOSTRING_H
#define TOSTRING_H

/** @todo Consider putting this file in another place, like a "shared" folder. */

/// Translates a element/register usage to a string
string opcode2str(D3DSHADER_INSTRUCTION_OPCODE_TYPE opc);
string registertype2string(D3DSHADER_PARAM_REGISTER_TYPE type);
string renderstate2string(D3DRENDERSTATETYPE state);
string samplerstate2string(D3DSAMPLERSTATETYPE state);

#endif
