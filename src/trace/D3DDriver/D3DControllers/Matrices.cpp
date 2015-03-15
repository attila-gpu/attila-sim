#include "Common.h"
#include "Matrices.h"

void d3d_identity_matrix(D3DMATRIX *dest) {
    for(u32bit i = 0; i < 4; i ++)
    for(u32bit j = 0; j < 4; j ++)
        if(i == j ) dest->m[i][j] = 1.0f;
        else dest->m[i][j] = 0.0f;
}

void d3d_translate_matrix(D3DMATRIX* dest, D3DVECTOR* offset) {
    d3d_identity_matrix(dest);
    dest->m[0][3] = offset->x;
    dest->m[1][3] = offset->y;
    dest->m[2][3] = offset->z;
}

void d3d_scale_matrix(D3DMATRIX* dest, D3DVECTOR* factor) {
    d3d_identity_matrix(dest);
    dest->m[0][0] = factor->x; // Great comic!
    dest->m[1][1] = factor->y;
    dest->m[2][2] = factor->z;
}


void d3d_copy_matrix(D3DMATRIX *dest, const D3DMATRIX* source) {
    for(u32bit i = 0; i < 4; i ++)
    for(u32bit j = 0; j < 4; j ++)
        dest->m[i][j] = source->m[i][j];
}

void d3d_multiply_matrix(D3DMATRIX *dest, const D3DMATRIX* source_a, const D3DMATRIX* source_b) {
    D3DMATRIX temp;
    for(u32bit i = 0; i < 4; i ++)
        for(u32bit j = 0; j < 4; j ++) {
            float dot = 0;
            for(u32bit k = 0; k < 4; k ++)
                dot += source_a->m[i][k] * source_b->m[k][j];
            temp.m[i][j] = dot;
        }
    d3d_copy_matrix(dest, &temp);
}

void d3d_transpose_matrix(D3DMATRIX *dest, const D3DMATRIX *source) {
    D3DMATRIX temp;
    for(u32bit i = 0; i < 4; i ++)
        for(u32bit j = 0; j < 4; j ++)
            temp.m[j][i] = source->m[i][j];
    d3d_copy_matrix(dest, &temp);
}

void d3d_invert_matrix(D3DMATRIX *dest, const D3DMATRIX *source) {
  // Thanks to gmt@aviator.cis.ufl.edu
  D3DMATRIX temp;

  float Tx, Ty, Tz;
  temp.m[0][0] = source->m[0][0];
  temp.m[1][0] = source->m[0][1];
  temp.m[2][0] = source->m[0][2];

  temp.m[0][1] = source->m[1][0];
  temp.m[1][1] = source->m[1][1];
  temp.m[2][1] = source->m[1][2];

  temp.m[0][2] = source->m[2][0];
  temp.m[1][2] = source->m[2][1];
  temp.m[2][2] = source->m[2][2];

  temp.m[0][3] = temp.m[1][3] = temp.m[2][3] = 0;
  temp.m[3][3] = 1;

  Tx = source->m[3][0];
  Ty = source->m[3][1];
  Tz = source->m[3][2];

  temp.m[3][0] = -( source->m[0][0] * Tx + source->m[0][1] * Ty + source->m[0][2] * Tz );
  temp.m[3][1] = -( source->m[1][0] * Tx + source->m[1][1] * Ty + source->m[1][2] * Tz );
  temp.m[3][2] = -( source->m[2][0] * Tx + source->m[2][1] * Ty + source->m[2][2] * Tz );

  d3d_copy_matrix(dest, &temp);
}

