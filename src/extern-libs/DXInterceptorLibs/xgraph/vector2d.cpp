#include "stdafx.h"
#include "vector2d.h"
#include <math.h>


PVECTOR2D   vSubtractVectors(PVECTOR2D v0, PVECTOR2D v1, PVECTOR2D v)
{
  if (v0 == NULL || v1 == NULL)
    v = (PVECTOR2D)NULL;
  else
  {
    v->x = v0->x - v1->x;
    v->y = v0->y - v1->y;
  }
  return(v);
}


PVECTOR2D   vAddVectors(PVECTOR2D v0, PVECTOR2D v1, PVECTOR2D v)
{
  if (v0 == NULL || v1 == NULL)
    v = (PVECTOR2D)NULL;
  else
  {
    v->x = v0->x + v1->x;
    v->y = v0->y + v1->y;
  }
  return(v);
}

PVECTOR2D   vScaleVector(PVECTOR2D v0, double dScaling, PVECTOR2D v)
{
  if (v0 == NULL)
    v = (PVECTOR2D)NULL;
  else
  {
    if (dScaling != 0)
    {
      v->x = (v0->x *= dScaling);
      v->y = (v0->y *= dScaling);
	}
  }
  return(v);
}

PVECTOR2D   vLinearCombination(PVECTOR2D vScale, PVECTOR2D v0, PVECTOR2D v1, PVECTOR2D v)
{
  if (vScale == NULL || v0 == NULL || v1 == NULL)
    v = (PVECTOR2D)NULL;
  else
  {
    v->x = vScale->x * v0->x + vScale->y * v1->x;
    v->y = vScale->x * v0->y + vScale->y * v1->y;
  }
  return(v);
}


double   vVectorSquared(PVECTOR2D v0)
{
  double dSqLen;

  if (v0 == NULL)
    dSqLen = 0.0;
  else
    dSqLen = (double)(v0->x * v0->x) + (double)(v0->y * v0->y);
  return (dSqLen);
}


double   vVectorMagnitude(PVECTOR2D v0)
{
  double dMagnitude;

  if (v0 == NULL)
    dMagnitude = 0.0;
  else
    dMagnitude = sqrt(vVectorSquared(v0));
  return (dMagnitude);
}


void   vNormalizeVector(PVECTOR2D v0)
{
  double dMagnitude = vVectorMagnitude(v0);

  v0->x /= dMagnitude;
  v0->y /= dMagnitude;
}


double   vDotProduct(PVECTOR2D v0, PVECTOR2D v1)
{
  return ((v0 == NULL || v1 == NULL) ? 0.0 
                                     : (v0->x * v1->x) + (v0->y * v1->y));
}

PVECTOR2D   vNormalVector(PVECTOR2D v0, PVECTOR2D v)
{
  if (v0 == NULL)
    v = (PVECTOR2D)NULL;
  else
  {
    v->x = -v0->y;
    v->y = v0->x;
  }
  return(v);
}

double   vVectorAngle(PVECTOR2D v0, PVECTOR2D v1)
{
  double vangle;

  if (v0 == NULL || v1 == NULL)
    vangle = 0.0;
  else
  {
    vNormalizeVector(v0);
    vNormalizeVector(v1);
	vangle = vDotProduct(v0, v1);
  }
  return(vangle);
}

BOOL   vPointNormalForm(POINT pt0, POINT pt1, PPOINTNORMAL ppnPointNormal)
{
  VECTOR2D v, vNormal;

  POINTS2VECTOR2D(pt0, pt1, v);

  if (v.x == 0 && v.y == 0)
    return(FALSE);

  vNormalVector(&v, &vNormal);

  ppnPointNormal->vNormal.x = vNormal.x;
  ppnPointNormal->vNormal.y = vNormal.y;
  ppnPointNormal->D = vDotProduct(&vNormal, (PVECTOR2D)&pt0);
  return(TRUE);
}

void   vProjectAndResolve(PVECTOR2D v0, PVECTOR2D v1, PPROJECTION ppProj)
{
  VECTOR2D ttProjection, ttOrthogonal;
  double proj1;
  //
  //obtain projection vector
  //
  //c = a * b
  //    ----- b
  //    |b|^2
  //
  proj1 = vDotProduct(v0, v1)/vDotProduct(v1, v1);
  ttProjection.x = v1->x * proj1;
  ttProjection.y = v1->y * proj1;
  //
  //obtain perpendicular projection : e = a - c
  //
  vSubtractVectors(v0, &ttProjection, &ttOrthogonal);
  //
  //fill PROJECTION structure with appropriate values
  //
  ppProj->LenProjection = vVectorMagnitude(&ttProjection);
  ppProj->LenPerpProjection = vVectorMagnitude(&ttOrthogonal);

  ppProj->ttProjection.x = ttProjection.x;
  ppProj->ttProjection.y = ttProjection.y;
  ppProj->ttPerpProjection.x = ttOrthogonal.x;
  ppProj->ttPerpProjection.y = ttOrthogonal.y;
}

BOOL   vIsPerpendicular(PVECTOR2D v0, PVECTOR2D v1)
{
  double product;

  if (v0 == NULL || v1 == NULL)
    product = 0.0;
  else
    product = vDotProduct(v0, v1);

  return (((product == 0.0) ? TRUE : FALSE));
}


double   vDistFromPointToLine(LPPOINT pt0, LPPOINT pt1, LPPOINT ptTest)
{
  VECTOR2D ttLine, ttTest;
  PROJECTION pProjection;

  POINTS2VECTOR2D(*pt0, *pt1, ttLine);
  POINTS2VECTOR2D(*pt0, *ptTest, ttTest);

  vProjectAndResolve(&ttTest, &ttLine, &pProjection);
 
  return(pProjection.LenPerpProjection);
}

