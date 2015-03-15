/**************************************************************************
 *
 * Copyright (c) 2002 - 2011 by Computer Architecture Department,
 * Universitat Politecnica de Catalunya.
 * All rights reserved.
 *
 * The contents of this file may not be disclosed to third parties,
 * copied or duplicated in any form, in whole or in part, without the
 * prior permission of the authors, Computer Architecture Department
 * and Universitat Politecnica de Catalunya.
 *
 */

#include "D3D9RenderTargetStats.h"
#include "D3D9State.h"
#include "OptimizationDataStructures.h"

#include <iostream>
#include <sstream>
#include <string>

using namespace std;

D3D9RenderTargetStats::D3D9RenderTargetStats () {
	width = 0;
	height = 0;
	lockable = 0;
	numRenderTargetTexture = 0;
	numDiffRenderTargetTexture = 0;
	size = 0;
	acumRenderTargets = 0;
    textureIdent = 0;
    backBuffer = false;

	for(int i = 0; i<9; i++) renderTargetType[i] = 0;
		differentRenderTarget.clear();
}

void D3D9RenderTargetStats::reset() {
	width = 0;
	height = 0;
	lockable = 0;
	numRenderTargetTexture = 0;
	numDiffRenderTargetTexture = 0;
	size = 0;
	acumRenderTargets = 0;
    textureIdent = 0;

	for(int i = 0; i<9; i++) renderTargetType[i] = 0;
		differentRenderTarget.clear();
}

void D3D9RenderTargetStats::addRenderTarget(UINT _Width,UINT _Height,D3DFORMAT _Format,D3DMULTISAMPLE_TYPE _MultiSample,DWORD _MultisampleQuality,
											BOOL _Lockable, IDirect3DSurface9* oip_ppSurface, /*IDirect3DSurface9** _ppSurface,*/ HANDLE* _pSharedHandle) {

	width = _Width;
	height = _Height;
	format = _Format;
	multiSample = _MultiSample;
	multisampleQuality = _MultisampleQuality;
	lockable = _Lockable;

	renderTargetType[D3D9TextureStats::getTexelSize(_Format)]++;
	renderTargetType2U[D3D9TextureStats::getTextureFormat(_Format)]++;

	acumRenderTargets++;
	differentRenderTarget.push_back(oip_ppSurface);

}

void D3D9RenderTargetStats::addRenderTargetStats(D3D9RenderTargetStats* second) {

	width += second->width;
	height += second->height;

	renderTargetType[0] += second->renderTargetType[0];
	renderTargetType[1] += second->renderTargetType[1];
	renderTargetType[2] += second->renderTargetType[2];
	renderTargetType[3] += second->renderTargetType[3];
	renderTargetType[4] += second->renderTargetType[4];
	renderTargetType[5] += second->renderTargetType[5];
	renderTargetType[6] += second->renderTargetType[6];
	renderTargetType[7] += second->renderTargetType[7];
	renderTargetType[8] += second->renderTargetType[8];

	numRenderTargetTexture += second->numRenderTargetTexture;
	acumRenderTargets += second->acumRenderTargets;

	list<IDirect3DSurface9 *>::iterator it;
	list <IDirect3DSurface9 *> aux;
	for ( it=(second->differentRenderTarget).begin() ; it != (second->differentRenderTarget).end(); it++ ) 
		aux.push_back(*it);

	differentRenderTarget.merge(aux);
	differentRenderTarget.unique();

	list<D3D9RenderTargetStats *> auxo = D3D9State::instance().getDifferentRenderTargetStats(differentRenderTarget);
	list<D3D9RenderTargetStats *> :: iterator ito;

	size = 0;
	numDiffRenderTargetTexture = 0;

	for (ito=auxo.begin(); ito!=auxo.end(); ito ++) {
		size += (*ito)->getSize();
		numDiffRenderTargetTexture += (*ito)->getNumDiffRenderTargetTexture();
        for (int i = 0; i < 61; i++)
            renderTargetType2U[i] += (*ito)->getRenderTargetType(i);
	}

}

int D3D9RenderTargetStats::getNumDiffRenderTargetTexture() {
	return numDiffRenderTargetTexture;
}

void D3D9RenderTargetStats::setTextureIdent(unsigned int ti) {
	textureIdent = ti;
}

unsigned int D3D9RenderTargetStats::getTextureIdent() {
	return textureIdent;
}

int D3D9RenderTargetStats::getRenderTargetType(int i) {
	return renderTargetType2U[i];
}

void D3D9RenderTargetStats::setSize(int _size) {
	size = _size;
}


int D3D9RenderTargetStats::getSize() {
	return size;
}

unsigned int D3D9RenderTargetStats::getWidth(){
    return width;
}

unsigned int D3D9RenderTargetStats::getHeight(){
    return height;
}

void D3D9RenderTargetStats::setDesc(D3DSURFACE_DESC* pDesc, IDirect3DSurface9* surface) {
	width = pDesc->Width;
	height = pDesc->Height;

	renderTargetType[D3D9TextureStats::getTexelSize(pDesc->Format)]++;
	acumRenderTargets++;
	differentRenderTarget.push_back(surface);
}

void D3D9RenderTargetStats::printRenderTargetStats (ofstream* f) {

	*f << differentRenderTarget.size() << ";";
	*f << width << ";";
	*f << height << ";";
	*f << size << ";";
	*f << numRenderTargetTexture << ";";
	*f << numDiffRenderTargetTexture << ";";
	*f << renderTargetType[0] << ";";
	*f << renderTargetType[1] << ";";
	*f << renderTargetType[2] << ";";
	*f << renderTargetType[3] << ";";
	*f << renderTargetType[4] << ";";
	*f << renderTargetType[5] << ";";
	*f << renderTargetType[6] << ";";
	*f << renderTargetType[7] << ";";
	*f << renderTargetType[8] << ";";
    
    for (int i = 0; i < 61; i++)
        *f << renderTargetType2U[i] << ";";
}

void D3D9RenderTargetStats::copy (D3D9TextureStats* second, IDirect3DSurface9* surface) {
	
	width = second->getWidth();
	height = second->getHeight();
	renderTargetType[0] += second->getTextureType(0);
	renderTargetType[1] += second->getTextureType(1);
	renderTargetType[2] += second->getTextureType(2);
	renderTargetType[3] += second->getTextureType(3);
	renderTargetType[4] += second->getTextureType(4);
	renderTargetType[5] += second->getTextureType(5);
	renderTargetType[6] += second->getTextureType(6);
	renderTargetType[7] += second->getTextureType(7);
	renderTargetType[8] += second->getTextureType(8);
	acumRenderTargets++;
	numRenderTargetTexture++;
	numDiffRenderTargetTexture++;
	differentRenderTarget.push_back(surface);
}

void D3D9RenderTargetStats::setBackBuffer(bool bb) {
    backBuffer = bb;
}

bool D3D9RenderTargetStats::isBackBuffer(){
    return backBuffer;
}

D3D9RenderTargetOp::D3D9RenderTargetOp(unsigned int i) {

    ident = i;
    draws = 0;
    numPixelsDrawn = 0;
    numClears = 0;
    numPartialClears = 0;
    dRT = false;
    stretchrect = false;
    colorWriteEnable = false;
    clearNode = false;
    fullStretchRect = false;
    //textIdentOp.push_back(D3D9State::instance().getTexIformRT(i));

}

void D3D9RenderTargetOp::addTextOp(unsigned int t) {

    textIdentOp.push_back(t);
    //draws++;

}

void D3D9RenderTargetOp::clear() {

    //textIdentOp.clear();
    numClears++;

}

void D3D9RenderTargetOp::partialClear() {

    //textIdentOp.clear();
    //numClears++;
    numPartialClears++;

}

void D3D9RenderTargetOp::save(ofstream* f) {

    textIdentOp.sort();
    textIdentOp.unique();

    std::list<unsigned int>::iterator itTIOp;

    if (stretchrect) *f << "SR ";
    else if (dRT) *f << "ZST ";
    else if (D3D9State::instance().isRTBackBuffer(ident)) *f << "BB ";
    else *f << "RTT ";

    *f << ident << " (" << D3D9State::instance().getTexIformRT(ident) << ") <--";

    for (itTIOp = textIdentOp.begin(); itTIOp != textIdentOp.end(); itTIOp++) {
        *f << " " << (*itTIOp);
    }

    *f << "\n";
}

std::string D3D9RenderTargetOp::getString() {

    std::ostringstream oss;

    /*if (stretchrect) {
        oss << "SR";
    }
    else {*/

        if (dRT) {

            oss << "ZST ";

            oss << ident;

        }
        else if (D3D9State::instance().isRTBackBuffer(ident)) {

            oss << "BB ";

            oss << ident;

        }
        else {

            oss << "RTT ";

            oss << ident;

        }

    //}

    return oss.str();

}

std::string D3D9RenderTargetOp::getExtraDescription() {

    std::ostringstream oss;

    if (stretchrect) {

        if (dRT) {

            oss << "ZST ";

            oss << ident;

        }
        else if (D3D9State::instance().isRTBackBuffer(ident)) {

            oss << "BB ";

            oss << ident;

        }
        else {

            oss << "RTT ";

            oss << ident;

        }

            D3D9RenderTargetStats* RTstats = D3D9State::instance().getRenderTargetStatsIdent(ident);

            if (RTstats != NULL) {
                oss << "\\n ";
                oss << RTstats->getWidth() << "x" << RTstats->getHeight();
            }

            oss << "\\n ";
            oss << "Pixels: " << numPixelsDrawn;

    }
    else {

        if (dRT) {

            D3D9RenderTargetStats* RTstats = D3D9State::instance().getRenderTargetStatsIdent(ident);

            if (RTstats != NULL) {
                //oss << "\\n ";
                oss << RTstats->getWidth() << "x" << RTstats->getHeight();
            }

            oss << "\\n ";
            oss << "Draws: " << draws;

            oss << "\\n ";
            oss << "Pixels: " << numPixelsDrawn;

            oss << "\\n ";
            oss << "P. Clears: " << numPartialClears;

        }
        else if (D3D9State::instance().isRTBackBuffer(ident)) {

            D3D9RenderTargetStats* RTstats = D3D9State::instance().getRenderTargetStatsIdent(ident);

            if (RTstats != NULL) {
                //oss << "\\n ";
                oss << RTstats->getWidth() << "x" << RTstats->getHeight();
            }

            oss << "\\n ";
            oss << "Draws: " << draws;

            oss << "\\n ";
            oss << "Pixels: " << numPixelsDrawn;

            oss << "\\n ";
            oss << "P. Clears: " << numPartialClears;

        }
        else {

            D3D9RenderTargetStats* RTstats = D3D9State::instance().getRenderTargetStatsIdent(ident);

            if (RTstats != NULL) {
                //oss << "\\n ";
                oss << RTstats->getWidth() << "x" << RTstats->getHeight();
            }

            oss << "\\n ";
            oss << "Draws: " << draws;

            oss << "\\n ";
            oss << "Pixels: " << numPixelsDrawn;

            oss << "\\n ";
            oss << "P. Clears: " << numPartialClears;

        }

    }

    return oss.str();

}

bool D3D9RenderTargetOp::isRT(unsigned int rt) {

    return (rt == ident);

}

bool D3D9RenderTargetOp::isZST(){
    return dRT;
}

unsigned int D3D9RenderTargetOp::getRT() {
    return ident;
}

unsigned int D3D9RenderTargetOp::getDraws() {
    return draws;
}

unsigned int D3D9RenderTargetOp::getClears() {
    return numClears;
}

void D3D9RenderTargetOp::addDraw(unsigned int d) {
    draws += d;
}

void D3D9RenderTargetOp::addNumPixelsDrawn(UINT64 num) {
    numPixelsDrawn += num;
}

UINT64 D3D9RenderTargetOp::getPixelsDrawn(){
    return numPixelsDrawn;
}

UINT64 D3D9RenderTargetOp::getSize() {

    UINT64 s = 0;

    D3D9RenderTargetStats* RTstats = D3D9State::instance().getRenderTargetStatsIdent(ident);

    if (RTstats != NULL)
        s = RTstats->getWidth() * RTstats->getHeight();

    return s;

}

std::list<unsigned int> D3D9RenderTargetOp::getOpList() {
    return textIdentOp;
}

void D3D9RenderTargetOp::depthRT(bool b) {
    dRT = b;
}


void D3D9RenderTargetOp::setStretchRect(bool b) {

    stretchrect = b;

}

bool D3D9RenderTargetOp::isStretchRect() {

    return stretchrect;

}

void D3D9RenderTargetOp::setColorWriteEnable(bool e){

    if (!colorWriteEnable)
        colorWriteEnable = e;
}

bool D3D9RenderTargetOp::getColorWriteEnable() {

    return colorWriteEnable;

}

void D3D9RenderTargetOp::setClearNode(bool c){

    clearNode = c;
}

bool D3D9RenderTargetOp::getClearNode() {

    return clearNode;

}

void D3D9RenderTargetOp::fullSR(bool fsr){
    fullStretchRect = fsr;
}

bool D3D9RenderTargetOp::getFullSR(){
    return fullStretchRect;
}