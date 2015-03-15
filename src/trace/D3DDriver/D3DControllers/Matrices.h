#ifndef MATRICES_H_INCLUDED
#define MATRICES_H_INCLUDED

void d3d_identity_matrix(D3DMATRIX* dest);
void d3d_translate_matrix(D3DMATRIX* dest, D3DVECTOR* offset);
void d3d_scale_matrix(D3DMATRIX* dest, D3DVECTOR* factor);

void d3d_copy_matrix(D3DMATRIX *dest, const D3DMATRIX* source);
void d3d_multiply_matrix(D3DMATRIX *dest, const D3DMATRIX* source_a, const D3DMATRIX* source_b);
void d3d_transpose_matrix(D3DMATRIX *dest, const D3DMATRIX *source);
void d3d_invert_matrix(D3DMATRIX *dest, const D3DMATRIX *source);

#endif
