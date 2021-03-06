/*
 Bulllord Game Engine
 Copyright (C) 2010-2017 Trix
 
 This software is provided 'as-is', without any express or implied
 warranty.  In no event will the authors be held liable for any damages
 arising from the use of this software.
 
 Permission is granted to anyone to use this software for any purpose,
 including commercial applications, and to alter it and redistribute it
 freely, subject to the following restrictions:
 
 1. The origin of this software must not be misrepresented; you must not
 claim that you wrote the original software. If you use this software
 in a product, an acknowledgment in the product documentation would be
 appreciated but is not required.
 2. Altered source versions must be plainly marked as such, and must not be
 misrepresented as being the original software.
 3. This notice may not be removed or altered from any source distribution.
 */
#include "../headers/sprite.h"
#include "../headers/gpu.h"
#include "../headers/system.h"
#include "../headers/utils.h"
#include "../headers/ui.h"
#include "internal/dictionary.h"
#include "internal/array.h"
#include "internal/internal.h"
#include "internal/mathematics.h"
typedef struct _SpriteAction{
    struct _SpriteAction* pNext;
    struct _SpriteAction* pNeighbor;
	BLBool bLoop;
	BLBool bParallel;
	BLF32 fCurTime;
	BLF32 fTotalTime;
	BLEnum eActionType;
	union {
		struct _Move {
			BLF32* pXPath;
			BLF32* pYPath;
			BLU32 nPathNum;
			BLF32 fAcceleration;
			BLF32 fVelocity;
		}sMove;
		struct _Rotate {
			BLF32 fAngle;
            BLBool bClockWise;
		}sRotate;
		struct _Scale {
			BLF32 fXInitScale;
			BLF32 fYInitScale;
			BLF32 fXScale;
			BLF32 fYScale;
            BLBool bReverse;
		}sScale;
		struct _ZValue {
			BLF32 fZv;
		}sZValue;
		struct _Alpha {
			BLF32 fInitAlpha;
			BLF32 fAlpha;
			BLBool bReverse;
		}sAlpha;
		struct _Dead {
			BLBool bDead;
		}sDead;
	} uAction;
}_BLSpriteAction;
typedef struct _EmitParam {
	BLF32 fEmitterRadius;
	BLU32 nLife;
	BLU32 nMaxAlive;
    BLU32 nCurAlive;
	BLF32 fGenPerMSec;
	BLF32 fDirectionX;
	BLF32 fDirectionY;
    BLF32 fVelocity;
    BLF32 fVelVariance;
	BLF32 fEmitAngle;
    BLF32 fRotation;
    BLF32 fRotVariance;
    BLF32 fScale;
    BLF32 fScaleVariance;
    BLU32 nFadeColor;
    BLU32* pAge;
    BLF32* pPositionX;
    BLF32* pPositionY;
    BLF32* pEmitAngle;
    BLF32* pVelocity;
    BLU32* pRotScale;
}_BLEmitParam;
typedef struct _SpriteSheet{
	BLU32 nTag;
	BLU32 nLTx;
	BLU32 nLTy;
	BLU32 nRBx;
	BLU32 nRBy;
	BLU32 nOffsetX;
	BLU32 nOffsetY;
}_BLSpriteSheet;
typedef struct _TileInfo{
	BLGuid nID;
	BLAnsi aFilename[260];
	BLAnsi aArchive[260];
	BLGuid nTex;
	BLVec2 sSize;
	BLVec2 sPos;
	BLF32 fTexLTx;
	BLF32 fTexLTy;
	BLF32 fTexRBx;
	BLF32 fTexRBy;
	BLBool bFlipX;
	BLBool bFlipY;
	BLBool bFlipD;
	BLBool bShow;
}_BLTileInfo;
typedef struct _SpriteNode{
    struct _SpriteNode* pParent;
    struct _SpriteNode** pChildren;
	_BLSpriteAction* pAction;
	_BLSpriteAction* pCurAction;
	_BLEmitParam* pEmitParam;
	BLDictionary* pTagSheet;
	BLU8* pTexData;
	BLEnum eTexFormat;
	BLU32 nTexWidth;
	BLU32 nTexHeight;
    BLGuid nID;
    BLU32 aTag[64];
	BLU32 nFrameNum;
	BLU32 nCurFrame;
	BLU32 nTimePassed;
	BLAnsi aFilename[260];
	BLAnsi aArchive[260];
    BLGuid nGBO;
    BLGuid nTex;
    BLVec2 sSize;
    BLVec2 sPos;
    BLVec2 sPivot;
    BLRect sScissor;
    BLU32 nFPS;
    BLF32 fScaleX;
    BLF32 fScaleY;
    BLF32 fRotate;
    BLF32 fZValue;
    BLF32 fAlpha;
    BLU32 nDyeColor;
    BLBool bDye;
	BLBool bFlipX;
	BLBool bFlipY;
    BLU32 nStrokeColor;
    BLU32 nStrokePixel;
    BLU32 nGlowColor;
    BLU32 nBrightness;
    BLU32 nChildren;
	BLBool bValid;
    BLBool bShow;
}_BLSpriteNode;
typedef struct _SpriteMember {
    BLGuid nSpriteTech;
    BLGuid nSpriteInstTech;
	BLGuid nSpriteStrokeTech;
	BLGuid nSpriteGlowTech;
    _BLSpriteNode** pNodeList;
	_BLSpriteNode* pCursor;
	BLArray* pTileArray[8];
	BLRect sViewport;
	BLBool bParallelEdit;
    BLU32 nNodeNum;
    BLU32 nNodeCap;
    BLBool bShaking;
    BLF32 fShakingTime;
    BLBool bShakingVertical;
    BLF32 fShakingForce;
}_BLSpriteMember;
static _BLSpriteMember* _PrSpriteMem = NULL;
extern BLBool _FetchResource(const BLAnsi*, const BLAnsi*, BLVoid**, BLGuid, BLBool(*)(BLVoid*, const BLAnsi*, const BLAnsi*), BLBool(*)(BLVoid*), BLBool);
extern BLBool _DiscardResource(BLGuid, BLBool(*)(BLVoid*), BLBool(*)(BLVoid*));
BLBool
_UseCustomCursor()
{
	if (_PrSpriteMem)
		return _PrSpriteMem->pCursor ? TRUE : FALSE;
	else
		return FALSE;
}
static const BLVoid
_MouseSubscriber(BLEnum _Type, BLU32 _UParam, BLS32 _SParam, BLVoid* _PParam)
{
	if (_Type == BL_ME_MOVE)
	{
        if (_PrSpriteMem->pCursor)
        {
            _PrSpriteMem->pCursor->sPos.fX = LOWU16(_UParam);
            _PrSpriteMem->pCursor->sPos.fY = HIGU16(_UParam);
        }
	}
}
static BLVoid
_AddToNodeList(_BLSpriteNode* _Node)
{
    BLS32 _mid = -1, _left = 0, _right = _PrSpriteMem->nNodeNum;
    while (_left < _right)
    {
        _mid = (_left + _right) >> 1;
        if (_PrSpriteMem->pNodeList[_mid]->fZValue > _Node->fZValue)
            _right = _mid;
        else if (_PrSpriteMem->pNodeList[_mid]->fZValue < _Node->fZValue)
        {
            _left = _mid;
            if (_right - _left == 1)
            {
                _mid = _right;
                break;
            }
        }
        else
            break;
    }
    if (_mid != -1)
    {
        if (_PrSpriteMem->nNodeCap <= _PrSpriteMem->nNodeNum)
        {
            _PrSpriteMem->nNodeCap = _PrSpriteMem->nNodeNum + 1;
            _PrSpriteMem->pNodeList = (_BLSpriteNode**)realloc(_PrSpriteMem->pNodeList, _PrSpriteMem->nNodeCap * sizeof(_BLSpriteNode*));
        }
        memmove(_PrSpriteMem->pNodeList + _mid + 1, _PrSpriteMem->pNodeList + _mid, sizeof(_BLSpriteNode*) * (_PrSpriteMem->nNodeNum - _mid));
        _PrSpriteMem->nNodeNum++;
        _PrSpriteMem->pNodeList[_mid] = _Node;
    }
    else
    {
        assert(_PrSpriteMem->nNodeNum == 0);
        _PrSpriteMem->nNodeNum++;
        _PrSpriteMem->nNodeCap++;
        _PrSpriteMem->pNodeList = (_BLSpriteNode**)realloc(_PrSpriteMem->pNodeList, _PrSpriteMem->nNodeNum * sizeof(_BLSpriteNode*));
        _PrSpriteMem->pNodeList[0] = _Node;
    }
}
static BLVoid
_RemoveFromNodeList(_BLSpriteNode* _Node)
{
    BLBool _found = FALSE;
    BLS32 _mid = -1, _left = 0, _right = _PrSpriteMem->nNodeNum;
    while (_left < _right)
    {
        _mid = (_left + _right) >> 1;
        if (_PrSpriteMem->pNodeList[_mid]->fZValue > _Node->fZValue)
            _right = _mid;
        else if (_PrSpriteMem->pNodeList[_mid]->fZValue < _Node->fZValue)
            _left = _mid;
        else
        {
            if (_PrSpriteMem->pNodeList[_mid] == _Node)
            {
                _found = TRUE;
                break;
            }
            else
            {
                BLS32 _tmp = _mid;
                while (_PrSpriteMem->pNodeList[_tmp]->fZValue == _Node->fZValue)
                {
                    _tmp++;
                    if (_tmp >= (BLS32)_PrSpriteMem->nNodeNum)
                        break;
                    if (_PrSpriteMem->pNodeList[_tmp] == _Node)
                    {
                        _found = TRUE;
                        _mid = _tmp;
                        break;
                    }
                }
                _tmp = _mid;
                while (_PrSpriteMem->pNodeList[_tmp]->fZValue == _Node->fZValue)
                {
                    _tmp--;
                    if (_tmp < 0)
                        break;
                    if (_PrSpriteMem->pNodeList[_tmp] == _Node)
                    {
                        _found = TRUE;
                        _mid = _tmp;
                        break;
                    }
                }
                break;
            }
        }
    }
    if (_found)
    {
        memmove(_PrSpriteMem->pNodeList + _mid, _PrSpriteMem->pNodeList + 1 + _mid, sizeof(_BLSpriteNode*) * (_PrSpriteMem->nNodeNum - _mid - 1));
        _PrSpriteMem->nNodeNum--;
    }
}
static BLBool
_SpriteSetup(BLVoid* _Src)
{
    _BLSpriteNode* _node = (_BLSpriteNode*)_Src;
    if (_node != _PrSpriteMem->pCursor)
        _AddToNodeList(_node);
    BLEnum _semantic[] = { BL_SL_POSITION, BL_SL_COLOR0, BL_SL_TEXCOORD0 };
    BLEnum _decls[] = { BL_VD_FLOATX2, BL_VD_FLOATX4, BL_VD_FLOATX2 };
	BLF32 _rgba[4];
	blDeColor4F(_node->nDyeColor, _rgba);
    BLF32 _vbo[] = {
        -_node->sSize.fX * 0.5f,
        -_node->sSize.fY * 0.5f,
        _rgba[0],
        _rgba[1],
        _rgba[2],
        _node->fAlpha,
        0.f,
        0.f,
        _node->sSize.fX * 0.5f,
        -_node->sSize.fY * 0.5f,
        _rgba[0],
        _rgba[1],
        _rgba[2],
        _node->fAlpha,
        1.f,
        0.f,
        -_node->sSize.fX * 0.5f,
        _node->sSize.fY * 0.5f,
        _rgba[0],
        _rgba[1],
        _rgba[2],
        _node->fAlpha,
        0.f,
        1.f,
        _node->sSize.fX * 0.5f,
        _node->sSize.fY * 0.5f,
        _rgba[0],
        _rgba[1],
        _rgba[2],
        _node->fAlpha,
        1.f,
        1.f
    };
    _node->nGBO = blGenGeometryBuffer(URIPART_INTERNAL(_node->nID), BL_PT_TRIANGLESTRIP, TRUE, _semantic, _decls, 3, _vbo, sizeof(_vbo), NULL, 0, BL_IF_INVALID);
    if (_node->pEmitParam)
    {
        BLEnum _semantice[] = { BL_SL_COLOR1, BL_SL_INSTANCE1 };
        BLEnum _decle[] = { BL_VD_FLOATX4, BL_VD_FLOATX4 };
		blGeometryBufferInstance(_node->nGBO, _semantice, _decle, 2, _node->pEmitParam->nMaxAlive);
    }
	_node->nTex = blGenTexture(blHashUtf8(_node->aFilename), BL_TT_2D, _node->eTexFormat, FALSE, TRUE, FALSE, 1, 1, _node->nTexWidth, _node->nTexHeight, 1, _node->pTexData);
	free(_node->pTexData);
	_node->pTexData = NULL;
    _node->bValid = TRUE;
    return TRUE;
}
static BLBool
_SpriteRelease(BLVoid* _Src)
{
    _BLSpriteNode* _node = (_BLSpriteNode*)_Src;
	if (_node->pParent)
	{
		BLBool _found = FALSE;
		for (BLU32 _idx = 0; _idx < _node->pParent->nChildren; ++_idx)
		{
			if (_node->pParent->pChildren[_idx] == _node)
			{
				memmove(_node->pParent->pChildren + _idx, _node->pParent->pChildren + _idx + 1, (_node->pParent->nChildren - _idx - 1) * sizeof(_BLSpriteNode*));
				_found = TRUE;
				break;
			}
		}
		if (!_found)
			return FALSE;
		_node->pParent->nChildren--;
		_node->pParent = NULL;
	}
	else
		_RemoveFromNodeList(_node);
    blDeleteGeometryBuffer(_node->nGBO);
	blDeleteTexture(_node->nTex);
    return TRUE;
}
static BLBool
_LoadSprite(BLVoid* _Src, const BLAnsi* _Filename, const BLAnsi* _Archive)
{
	_BLSpriteNode* _src = (_BLSpriteNode*)_Src;
	BLGuid _stream;
	if (_Archive)
		_stream = blGenStream(_Filename, _Archive);
	else
	{
		BLAnsi _tmpname[260];
		strcpy(_tmpname, _Filename);
		BLAnsi _path[260] = { 0 };
		strcpy(_path, blWorkingDir(TRUE));
		strcat(_path, _tmpname);
		_stream = blGenStream(_path, NULL);
		if (INVALID_GUID == _stream)
		{
			memset(_path, 0, sizeof(_path));
			strcpy(_path, blUserFolderDir());
			_stream = blGenStream(_path, NULL);
		}
		if (INVALID_GUID == _stream)
			return FALSE;
	}
	BLU8 _identifier[12];
	blStreamRead(_stream, sizeof(_identifier), _identifier);
	if (_identifier[0] != 0xDD ||
		_identifier[1] != 0xDD ||
		_identifier[2] != 0xDD ||
		_identifier[3] != 0xEE ||
		_identifier[4] != 0xEE ||
		_identifier[5] != 0xEE ||
		_identifier[6] != 0xAA ||
		_identifier[7] != 0xAA ||
		_identifier[8] != 0xAA ||
		_identifier[9] != 0xDD ||
		_identifier[10] != 0xDD ||
		_identifier[11] != 0xDD)
	{
		blDeleteStream(_stream);
		return FALSE;
	}
	BLU32 _width, _height, _depth;
	blStreamRead(_stream, sizeof(BLU32), &_width);
	blStreamRead(_stream, sizeof(BLU32), &_height);
	blStreamRead(_stream, sizeof(BLU32), &_depth);
	BLU32 _array, _faces, _mipmap;
	blStreamRead(_stream, sizeof(BLU32), &_array);
	blStreamRead(_stream, sizeof(BLU32), &_faces);
	blStreamRead(_stream, sizeof(BLU32), &_mipmap);
	BLU32 _fourcc, _channels, _offset;
	blStreamRead(_stream, sizeof(BLU32), &_fourcc);
	blStreamRead(_stream, sizeof(BLU32), &_channels);
	blStreamRead(_stream, sizeof(BLU32), &_offset);
	_src->nTexWidth = _width;
	_src->nTexHeight = _height;
	BLEnum _type;
	if (_height == 1)
		_type = BL_TT_1D;
	else
	{
		if (_depth == 1)
			_type = (_faces != 6) ? BL_TT_2D : BL_TT_CUBE;
		else
			_type = BL_TT_3D;
	}
	if (_type != BL_TT_2D)
	{
		blDeleteStream(_stream);
		return FALSE;
	}
	_src->pTagSheet = blGenDict(FALSE);
	while (blStreamTell(_stream) < _offset)
	{
		_BLSpriteSheet* _ss = (_BLSpriteSheet*)malloc(sizeof(_BLSpriteSheet));
		BLU32 _taglen;
		blStreamRead(_stream, sizeof(BLU32), &_taglen);
		BLAnsi _tag[256] = { 0 };
		blStreamRead(_stream, _taglen, _tag);
		_ss->nTag = blHashUtf8(_tag);
		blStreamRead(_stream, sizeof(BLU32), &_ss->nLTx);
		blStreamRead(_stream, sizeof(BLU32), &_ss->nLTy);
		blStreamRead(_stream, sizeof(BLU32), &_ss->nRBx);
		blStreamRead(_stream, sizeof(BLU32), &_ss->nRBy);
		blStreamRead(_stream, sizeof(BLU32), &_ss->nOffsetX);
		blStreamRead(_stream, sizeof(BLU32), &_ss->nOffsetY);
		blDictInsert(_src->pTagSheet, _ss->nTag, _ss);
		if (_src->nFrameNum == 1)
			_src->aTag[0] = _ss->nTag;
	}
	blStreamSeek(_stream, _offset);
	BLU32 _imagesz;
	switch (_fourcc)
	{
	case FOURCC_INTERNAL('B', 'M', 'G', 'T'):
		blStreamRead(_stream, sizeof(BLU32), &_imagesz);
		_src->eTexFormat = (_channels == 4) ? BL_TF_RGBA8 : BL_TF_RGB8;
		break;
	case FOURCC_INTERNAL('S', '3', 'T', '1'):
		_imagesz = ((_width + 3) / 4) * ((_height + 3) / 4) * 8;
		_src->eTexFormat = BL_TF_BC1;
		break;
	case FOURCC_INTERNAL('S', '3', 'T', '2'):
		_imagesz = ((_width + 3) / 4) * ((_height + 3) / 4) * 8;
		_src->eTexFormat = BL_TF_BC1A1;
		break;
	case FOURCC_INTERNAL('S', '3', 'T', '3'):
		_imagesz = ((_width + 3) / 4) * ((_height + 3) / 4) * 16;
		_src->eTexFormat = BL_TF_BC3;
		break;
	case FOURCC_INTERNAL('A', 'S', 'T', '1'):
		_imagesz = ((_width + 3) / 4) * ((_height + 3) / 4) * 16;
		_src->eTexFormat = BL_TF_ASTC;
		break;
	case FOURCC_INTERNAL('A', 'S', 'T', '2'):
		_imagesz = ((_width + 3) / 4) * ((_height + 3) / 4) * 16;
		_src->eTexFormat = BL_TF_ASTC;
		break;
	case FOURCC_INTERNAL('A', 'S', 'T', '3'):
		_imagesz = ((_width + 3) / 4) * ((_height + 3) / 4) * 16;
		_src->eTexFormat = BL_TF_ASTC;
		break;
	case FOURCC_INTERNAL('E', 'T', 'C', '1'):
		_imagesz = ((_width + 3) / 4) * ((_height + 3) / 4) * 8;
		_src->eTexFormat = BL_TF_ETC2;
		break;
	case FOURCC_INTERNAL('E', 'T', 'C', '2'):
		_imagesz = ((_width + 3) / 4) * ((_height + 3) / 4) * 8;
		_src->eTexFormat = BL_TF_ETC2A1;
		break;
	case FOURCC_INTERNAL('E', 'T', 'C', '3'):
		_imagesz = ((_width + 3) / 4) * ((_height + 3) / 4) * 16;
		_src->eTexFormat = BL_TF_ETC2A;
		break;
	default:assert(0); break;
	}
	if (_fourcc == FOURCC_INTERNAL('B', 'M', 'G', 'T'))
	{
		BLU8* _data = (BLU8*)malloc(_imagesz);
		blStreamRead(_stream, _imagesz, _data);
		BLU8* _data2 = (BLU8*)malloc(_width * _height * _channels);
		blRLEDecode(_data, _width * _height * _channels, _data2);
		free(_data);
		_src->pTexData = _data2;
	}
	else
	{
		BLU8* _data = (BLU8*)malloc(_imagesz);
		blStreamRead(_stream, _imagesz, _data);
		_src->pTexData = _data;
	}	
	blDeleteStream(_stream);
    return TRUE;
}
static BLBool
_UnloadSprite(BLVoid* _Src)
{
	_BLSpriteNode* _node = (_BLSpriteNode*)_Src;
	if (_node->pAction)
	{
		_BLSpriteAction* _tmp = _node->pAction;
		while (_tmp)
		{
			_BLSpriteAction* _tmpnext = _tmp->pNext;
			if (_tmp->pNeighbor)
			{
				_BLSpriteAction* _tmpneb = _tmp->pNeighbor;
				while (_tmp)
				{
					if (_tmp->eActionType == SPACTION_MOVE_INTERNAL)
					{
						free(_tmp->uAction.sMove.pXPath);
						free(_tmp->uAction.sMove.pYPath);
					}
					free(_tmp);
					_tmp = _tmpneb;
					_tmpneb = _tmp ? _tmp->pNeighbor : NULL;
				}
			}
			else
			{
				if (_tmp->eActionType == SPACTION_MOVE_INTERNAL)
				{
					free(_tmp->uAction.sMove.pXPath);
					free(_tmp->uAction.sMove.pYPath);
				}
				free(_tmp);
			}
			_tmp = _tmpnext;
		}
	}
	if (_node->pEmitParam)
    {
        free(_node->pEmitParam->pPositionX);
        free(_node->pEmitParam->pPositionY);
        free(_node->pEmitParam->pAge);
        free(_node->pEmitParam);
    }
	{
		FOREACH_DICT (_BLSpriteSheet*, _iter, _node->pTagSheet)
		{
			free(_iter);
		}
		blDeleteDict(_node->pTagSheet);
	}
	_node->bValid = FALSE;
    return TRUE;
}
static BLVoid
_SpriteUpdate(BLU32 _Delta)
{
	for (BLU32 _idx = 0; _idx < _PrSpriteMem->nNodeNum;)
	{
		BLBool _delete = FALSE;
		_BLSpriteNode* _node = _PrSpriteMem->pNodeList[_idx];
		if (_node->pCurAction)
		{
			_BLSpriteAction* _action = _node->pCurAction;
			do
			{
				_action->fCurTime += (BLF32)(_Delta) / 1000.f;
				switch (_action->eActionType)
				{
				case SPACTION_MOVE_INTERNAL:
				{
					BLF32 _s = _action->uAction.sMove.fVelocity * _action->fCurTime + 0.5f * _action->uAction.sMove.fAcceleration * _action->fCurTime * _action->fCurTime;
					for (BLU32 _jdx = 1; _jdx < _action->uAction.sMove.nPathNum; ++_jdx)
					{
						BLF32* _xp = _action->uAction.sMove.pXPath;
						BLF32* _yp = _action->uAction.sMove.pYPath;
						BLF32 _subs = sqrtf((_xp[_jdx] - _xp[_jdx - 1]) * (_xp[_jdx] - _xp[_jdx - 1]) + (_yp[_jdx] - _yp[_jdx - 1]) * (_yp[_jdx] - _yp[_jdx - 1]));
						if (_s > _subs)
							_s -= _subs;
						else
						{
							BLF32 _ratio = _s / _subs;
							_node->sPos.fX = _xp[_jdx - 1] + _ratio * (_xp[_jdx] - _xp[_jdx - 1]);
							_node->sPos.fY = _yp[_jdx - 1] + _ratio * (_yp[_jdx] - _yp[_jdx - 1]);
						}
                    }
				}
				break;
				case SPACTION_ROTATE_INTERNAL:
				{
					_node->fRotate = _action->fCurTime / _action->fTotalTime * _action->uAction.sRotate.fAngle * PI_INTERNAL / 180.0f;
                    _node->fRotate = _action->uAction.sRotate.bClockWise ? _node->fRotate : 2 * PI_INTERNAL - _node->fRotate;
				}
				break;
				case SPACTION_SCALE_INTERNAL:
				{
                    if (_action->uAction.sScale.bReverse)
                    {
                        if (_action->fCurTime * 2 <= _action->fTotalTime)
                        {
                            _node->fScaleX = 2 * _action->fCurTime / _action->fTotalTime * (_action->uAction.sScale.fXScale - _action->uAction.sScale.fXInitScale) + _action->uAction.sScale.fXInitScale;
                            _node->fScaleY = 2 * _action->fCurTime / _action->fTotalTime * (_action->uAction.sScale.fYScale - _action->uAction.sScale.fYInitScale) + _action->uAction.sScale.fYInitScale;
                        }
                        else
                        {
                            _node->fScaleX = (_action->fTotalTime - 2 * _action->fCurTime) / _action->fTotalTime * (_action->uAction.sScale.fXInitScale) + _action->uAction.sScale.fXScale;
                            _node->fScaleY = (_action->fTotalTime - 2 * _action->fCurTime) / _action->fTotalTime * (_action->uAction.sScale.fYInitScale) + _action->uAction.sScale.fYScale;
                        }
                    }
                    else
                    {
                        _node->fScaleX = _action->fCurTime / _action->fTotalTime * (_action->uAction.sScale.fXScale - _action->uAction.sScale.fXInitScale) + _action->uAction.sScale.fXInitScale;
                        _node->fScaleY = _action->fCurTime / _action->fTotalTime * (_action->uAction.sScale.fYScale - _action->uAction.sScale.fYInitScale) + _action->uAction.sScale.fYInitScale;
                    }
                }
				break;
				case SPACTION_ALPHA_INTERNAL:
				{
					if (_action->uAction.sAlpha.bReverse)
					{
						if (_action->fCurTime * 2 <= _action->fTotalTime)
							_node->fAlpha = 2 * _action->fCurTime / _action->fTotalTime * (_action->uAction.sAlpha.fAlpha - _action->uAction.sAlpha.fInitAlpha) + _action->uAction.sAlpha.fInitAlpha;
						else
							_node->fAlpha = (2 * _action->fCurTime - _action->fTotalTime) / _action->fTotalTime * (_action->uAction.sAlpha.fInitAlpha) + _action->uAction.sAlpha.fAlpha;
					}
					else
						_node->fAlpha = _action->fCurTime / _action->fTotalTime * (_action->uAction.sAlpha.fAlpha - _action->uAction.sAlpha.fInitAlpha) + _action->uAction.sAlpha.fInitAlpha;
				}
				break;
				case SPACTION_DEAD_INTERNAL:
				{
					blDeleteSprite(_node->nID);
					_delete = TRUE;
				}
				break;
				default:
					break;
				}
                if (_delete)
                    break;
				_action = _action->pNeighbor;
			} while (_action);
			if (_node->pCurAction->fCurTime >= _node->pCurAction->fTotalTime && !_delete)
			{
				if (!_node->pCurAction->bLoop)
				{
					if (!_node->pCurAction->pNext)
					{
						_BLSpriteAction* _tmp = _node->pAction;
						while (_tmp)
						{
							_BLSpriteAction* _tmpnext = _tmp->pNext;
							if (_tmp->pNeighbor)
							{
								_BLSpriteAction* _tmpneb = _tmp->pNeighbor;
								while (_tmp)
								{
									if (_tmp->eActionType == SPACTION_MOVE_INTERNAL)
									{
										free(_tmp->uAction.sMove.pXPath);
										free(_tmp->uAction.sMove.pYPath);
									}
									free(_tmp);
									_tmp = _tmpneb;
									_tmpneb = _tmp ? _tmp->pNeighbor : NULL;
								}
							}
							else
							{
								if (_tmp->eActionType == SPACTION_MOVE_INTERNAL)
								{
									free(_tmp->uAction.sMove.pXPath);
									free(_tmp->uAction.sMove.pYPath);
								}
								free(_tmp);
							}
							_tmp = _tmpnext;
						}
						_node->pAction = _node->pCurAction = NULL;
					}
					else
						_node->pCurAction = _node->pCurAction->pNext;
				}
				else
				{
					_node->pCurAction->fCurTime = 0.f;
					switch (_node->pCurAction->eActionType)
					{
					case SPACTION_MOVE_INTERNAL:
						{
							_node->sPos.fX = _node->pCurAction->uAction.sMove.pXPath[0];
							_node->sPos.fY = _node->pCurAction->uAction.sMove.pYPath[0];
						}
						break;
					case SPACTION_SCALE_INTERNAL:
						{
							_node->fScaleX = _node->pCurAction->uAction.sScale.fXInitScale;
							_node->fScaleY = _node->pCurAction->uAction.sScale.fYInitScale;
						}
						break;
					case SPACTION_ALPHA_INTERNAL:
						{
							_node->fAlpha = _node->pCurAction->uAction.sAlpha.fInitAlpha;
						}
						break;
					default:
						break;
					}
				}
			}
		}
		if (!_delete)
			_idx++;
	}
}
static BLVoid
_SpriteDraw(BLU32 _Delta, _BLSpriteNode* _Node, BLF32 _Mat[6])
{
    if (!_Node->bShow || !_Node->bValid)
        return;
	BLU32 _w, _h;
	blUIQueryResolution(&_w, &_h);
	if (_Node->sScissor.sLT.fX < 0.f)
		blRasterState(BL_CM_CW, 0, 0.f, TRUE, 0, 0, _w, _h);
	else
		blRasterState(BL_CM_CW, 0, 0.f, TRUE, (BLU32)_Node->sScissor.sLT.fX, (BLU32)_Node->sScissor.sLT.fY, (BLU32)_Node->sScissor.sRB.fX, (BLU32)_Node->sScissor.sRB.fY);
    if (_PrSpriteMem->bShaking)
    {
        _PrSpriteMem->fShakingForce = - _PrSpriteMem->fShakingForce;
        _PrSpriteMem->fShakingTime -= _Delta / 1000.f;
        if (_PrSpriteMem->fShakingTime < 0.f)
            _PrSpriteMem->bShaking = FALSE;
    }
	if (_Node->nFrameNum > 1)
	{
		_Node->nTimePassed += _Delta;
		if (_Node->nTimePassed >= 1000 / _Node->nFPS)
		{
			_Node->nTimePassed = 0;
			_Node->nCurFrame++;
			if (_Node->nCurFrame >= _Node->nFrameNum)
				_Node->nCurFrame = 0;
		}
	}
	_BLSpriteSheet* _ss = blDictElement(_Node->pTagSheet, _Node->aTag[_Node->nCurFrame]);
	BLF32 _minx, _miny, _maxx, _maxy;
	if (!_ss)
	{
		_minx = -_Node->sSize.fX * 0.5f;
		_miny = -_Node->sSize.fY * 0.5f;
		_maxx = _Node->sSize.fX * 0.5f;
		_maxy = _Node->sSize.fY * 0.5f;
	}
	else
	{
		BLF32 _dif = (BLF32)MAX_INTERNAL(_Node->nStrokePixel, 3);
		if (_Node->bFlipX)
		{
			_maxx = _Node->sSize.fX * 0.5f + _dif - _ss->nOffsetX;
			_minx = _maxx - _ss->nRBx + _ss->nLTx - _dif;
		}
		else
		{
			_minx = -_Node->sSize.fX * 0.5f + _ss->nOffsetX - _dif;
			_maxx = _minx + _ss->nRBx - _ss->nLTx + _dif;
		}
		if (_Node->bFlipY)
		{
			_maxy = _Node->sSize.fY * 0.5f + _dif - _ss->nOffsetY;
			_miny = _maxy - _ss->nRBy + _ss->nLTy - _dif;
		}
		else
		{
			_miny = -_Node->sSize.fY * 0.5f + _ss->nOffsetY - _dif;
			_maxy = _miny + _ss->nRBy - _ss->nLTy + _dif;
		}
	}
    BLF32 _ltx = (_minx * _Mat[0]) + (_miny * _Mat[2]) + _Mat[4];
    BLF32 _lty = (_minx * _Mat[1]) + (_miny * _Mat[3]) + _Mat[5];
    BLF32 _rtx = (_maxx * _Mat[0]) + (_miny * _Mat[2]) + _Mat[4];
    BLF32 _rty = (_maxx * _Mat[1]) + (_miny * _Mat[3]) + _Mat[5];
    BLF32 _lbx = (_minx * _Mat[0]) + (_maxy * _Mat[2]) + _Mat[4];
    BLF32 _lby = (_minx * _Mat[1]) + (_maxy * _Mat[3]) + _Mat[5];
    BLF32 _rbx = (_maxx * _Mat[0]) + (_maxy * _Mat[2]) + _Mat[4];
    BLF32 _rby = (_maxx * _Mat[1]) + (_maxy * _Mat[3]) + _Mat[5];
    if (_Node->pEmitParam)
    {
        BLU32 _gen = (BLU32)(_Node->pEmitParam->fGenPerMSec * _Delta);
        _Node->pEmitParam->nCurAlive = MIN_INTERNAL(_Node->pEmitParam->nMaxAlive, _gen + _Node->pEmitParam->nCurAlive);
        BLF32* _clrbuf = (BLF32*)alloca(_Node->pEmitParam->nMaxAlive * 4 * sizeof(BLF32));
        BLF32* _tranbuf = (BLF32*)alloca(_Node->pEmitParam->nMaxAlive * 4 * sizeof(BLF32));
        for (BLU32 _idx = 0; _idx < _Node->pEmitParam->nCurAlive; ++_idx)
        {
            if (_Node->pEmitParam->pAge[_idx] == 0)
            {
                _Node->pEmitParam->pPositionX[_idx] = _Node->sPos.fX + cosf(blRandRangeF(0.f , PI_INTERNAL * 2.f)) * _Node->pEmitParam->fEmitterRadius * blRandF();
                _Node->pEmitParam->pPositionY[_idx] = _Node->sPos.fY + sinf(blRandRangeF(0.f , PI_INTERNAL * 2.f)) * _Node->pEmitParam->fEmitterRadius * blRandF();
                _Node->pEmitParam->pEmitAngle[_idx] = blRandRangeF(-_Node->pEmitParam->fEmitAngle, _Node->pEmitParam->fEmitAngle);
                _Node->pEmitParam->pVelocity[_idx] = _Node->pEmitParam->fVelocity + blRandRangeF(-_Node->pEmitParam->fVelVariance, _Node->pEmitParam->fVelVariance);
                BLS32 _initrot = 100 * (BLS32)(_Node->pEmitParam->fRotation + blRandRangeF(-_Node->pEmitParam->fRotVariance, _Node->pEmitParam->fRotVariance));
                BLS32 _initscale = 100 * (BLS32)(_Node->pEmitParam->fScale + blRandRangeF(-_Node->pEmitParam->fScaleVariance, _Node->pEmitParam->fScaleVariance));
                _Node->pEmitParam->pRotScale[_idx] = MAKEU32(MAX_INTERNAL(0, _initrot), MAX_INTERNAL(0, _initscale));
            }
            else
            {
                BLF32 _s = _Node->pEmitParam->pVelocity[_idx] * _Delta / 1000.f;
                BLF32 _xvec, _yvec;
                _xvec = _Node->pEmitParam->fDirectionX * cosf(_Node->pEmitParam->pEmitAngle[_idx]) - _Node->pEmitParam->fDirectionY * sinf(_Node->pEmitParam->pEmitAngle[_idx]);
                _yvec = _Node->pEmitParam->fDirectionX * sinf(_Node->pEmitParam->pEmitAngle[_idx]) + _Node->pEmitParam->fDirectionY * cosf(_Node->pEmitParam->pEmitAngle[_idx]);
                _Node->pEmitParam->pPositionX[_idx] += _xvec * _s;
                _Node->pEmitParam->pPositionY[_idx] += _yvec * _s;
            }
            BLF32 _fadeclr[4], _dyeclr[4];
            blDeColor4F(_Node->pEmitParam->nFadeColor, _fadeclr);
            blDeColor4F(_Node->nDyeColor, _dyeclr);
            BLF32 _rot = (BLF32)HIGU16(_Node->pEmitParam->pRotScale[_idx]) / 100.f;
            BLF32 _scale = (BLF32)LOWU16(_Node->pEmitParam->pRotScale[_idx]) / 100.f;
            BLF32 _ratio = (BLF32)_Node->pEmitParam->pAge[_idx] / (BLF32)_Node->pEmitParam->nLife;
            _clrbuf[4 * _idx + 0] = (_fadeclr[0] - _dyeclr[0]) * _ratio + _dyeclr[0];
            _clrbuf[4 * _idx + 1] = (_fadeclr[1] - _dyeclr[1]) * _ratio + _dyeclr[1];
            _clrbuf[4 * _idx + 2] = (_fadeclr[2] - _dyeclr[2]) * _ratio + _dyeclr[2];
            _clrbuf[4 * _idx + 3] = (_fadeclr[3] - _dyeclr[3]) * _ratio + _dyeclr[3];
            _tranbuf[4 * _idx + 0] = _Node->pEmitParam->pPositionX[_idx];
            _tranbuf[4 * _idx + 1] = _Node->pEmitParam->pPositionY[_idx];
            _tranbuf[4 * _idx + 2] = _rot * _ratio;
            _tranbuf[4 * _idx + 3] = (_scale - 1.f) * _ratio + 1.f;
            _Node->pEmitParam->pAge[_idx] += _Delta;
        }
        if (_Node->pEmitParam->nCurAlive)
        {
            blInstanceUpdate(_Node->nGBO, BL_SL_COLOR1, _clrbuf, _Node->pEmitParam->nCurAlive * 4 * sizeof(BLF32));
            blInstanceUpdate(_Node->nGBO, BL_SL_INSTANCE1, _tranbuf, _Node->pEmitParam->nCurAlive * 4 * sizeof(BLF32));
        }
        for (BLU32 _idx = 0; _idx < _Node->pEmitParam->nMaxAlive; ++_idx)
        {
            if (_Node->pEmitParam->pAge[_idx] < _Node->pEmitParam->nLife)
            {
                memmove(_Node->pEmitParam->pAge, _Node->pEmitParam->pAge + _idx, (_Node->pEmitParam->nMaxAlive - _idx) * sizeof(BLU32));
                memmove(_Node->pEmitParam->pPositionX, _Node->pEmitParam->pPositionX + _idx, (_Node->pEmitParam->nMaxAlive - _idx) * sizeof(BLF32));
                memmove(_Node->pEmitParam->pPositionY, _Node->pEmitParam->pPositionY + _idx, (_Node->pEmitParam->nMaxAlive - _idx) * sizeof(BLF32));
                memmove(_Node->pEmitParam->pEmitAngle, _Node->pEmitParam->pEmitAngle + _idx, (_Node->pEmitParam->nMaxAlive - _idx) * sizeof(BLF32));
                memmove(_Node->pEmitParam->pVelocity, _Node->pEmitParam->pVelocity + _idx, (_Node->pEmitParam->nMaxAlive - _idx) * sizeof(BLF32));
                memmove(_Node->pEmitParam->pRotScale, _Node->pEmitParam->pRotScale + _idx, (_Node->pEmitParam->nMaxAlive - _idx) * sizeof(BLU32));
                memset(_Node->pEmitParam->pAge + _Node->pEmitParam->nMaxAlive - _idx, 0 , _idx * sizeof(BLU32));
                break;
            }
        }
		blTechSampler(_PrSpriteMem->nSpriteInstTech, "Texture0", _Node->nTex, 0);
    }
	else if (_Node->nBrightness > 0)
	{
		blTechSampler(_PrSpriteMem->nSpriteGlowTech, "Texture0", _Node->nTex, 0);
		BLF32 _glow[4];
		blDeColor4F(_Node->nGlowColor, _glow);
		_glow[3] = (BLF32)_Node->nBrightness;
		blTechUniform(_PrSpriteMem->nSpriteGlowTech, BL_UB_F32X4, "Glow", _glow, sizeof(_glow));
		BLF32 _texsize[] = { (BLF32)_Node->nTexWidth, (BLF32)_Node->nTexHeight };
		blTechUniform(_PrSpriteMem->nSpriteGlowTech, BL_UB_F32X2, "TexSize", _texsize, sizeof(_texsize));
		BLF32 _border[] = { (BLF32)_ss->nLTx, (BLF32)_ss->nLTy, (BLF32)_ss->nRBx, (BLF32)_ss->nRBy };
		blTechUniform(_PrSpriteMem->nSpriteGlowTech, BL_UB_F32X4, "Border", _border, sizeof(_border));
	}
	else if (_Node->nStrokePixel > 0)
	{
		blTechSampler(_PrSpriteMem->nSpriteStrokeTech, "Texture0", _Node->nTex, 0);
		BLF32 _stroke[4];
		blDeColor4F(_Node->nStrokeColor, _stroke);
		_stroke[3] = (BLF32)_Node->nStrokePixel;
		blTechUniform(_PrSpriteMem->nSpriteStrokeTech, BL_UB_F32X4, "Stroke", _stroke, sizeof(_stroke));
		BLF32 _texsize[] = { (BLF32)_Node->nTexWidth, (BLF32)_Node->nTexHeight };
		blTechUniform(_PrSpriteMem->nSpriteStrokeTech, BL_UB_F32X2, "TexSize", _texsize, sizeof(_texsize));
		BLF32 _border[] = { (BLF32)_ss->nLTx - (BLF32)_Node->nStrokePixel, (BLF32)_ss->nLTy - (BLF32)_Node->nStrokePixel, (BLF32)_ss->nRBx + (BLF32)_Node->nStrokePixel, (BLF32)_ss->nRBy + (BLF32)_Node->nStrokePixel };
		blTechUniform(_PrSpriteMem->nSpriteStrokeTech, BL_UB_F32X4, "Border", _border, sizeof(_border));
	}
	else
		blTechSampler(_PrSpriteMem->nSpriteTech, "Texture0", _Node->nTex, 0);
	if (_ss)
	{
		BLF32 _lttx, _ltty, _rttx, _rtty, _lbtx, _lbty, _rbtx, _rbty;
		BLS32 _dif = MAX_INTERNAL(_Node->nStrokePixel, 3);
		_lbtx = _lttx = (BLF32)((BLS32)_ss->nLTx - _dif) / (BLF32)_Node->nTexWidth;
		_rtty = _ltty = (BLF32)((BLS32)_ss->nLTy - _dif) / (BLF32)_Node->nTexHeight;
		_rbtx = _rttx = (BLF32)((BLS32)_ss->nRBx + _dif) / (BLF32)_Node->nTexWidth;
		_rbty = _lbty = (BLF32)((BLS32)_ss->nRBy + _dif) / (BLF32)_Node->nTexHeight;
		if (_Node->bFlipX)
		{ 
			BLF32 _tmp;
			_tmp = _lbtx;
			_lbtx = _lttx = _rbtx;
			_rbtx = _rttx = _tmp;
		}
		if (_Node->bFlipY)
		{
			BLF32 _tmp;
			_tmp = _ltty;
			_rtty = _ltty = _rbty;
			_rbty = _lbty = _tmp;
		}
		BLF32 _rgba[4];
		blDeColor4F(_Node->nDyeColor, _rgba);
		BLF32 _vbo[] = {
			_ltx + ((_PrSpriteMem->bShaking && !_PrSpriteMem->bShakingVertical) ? _PrSpriteMem->fShakingForce : 0.f),
			_lty + ((_PrSpriteMem->bShaking && _PrSpriteMem->bShakingVertical) ? _PrSpriteMem->fShakingForce : 0.f),
			_rgba[0],
			_rgba[1],
			_rgba[2],
			_Node->fAlpha,
			_lttx,
			_ltty,
			_rtx + ((_PrSpriteMem->bShaking && !_PrSpriteMem->bShakingVertical) ? _PrSpriteMem->fShakingForce : 0.f),
			_rty + ((_PrSpriteMem->bShaking && _PrSpriteMem->bShakingVertical) ? _PrSpriteMem->fShakingForce : 0.f),
			_rgba[0],
			_rgba[1],
			_rgba[2],
			_Node->fAlpha,
			_rttx,
			_rtty,
			_lbx + ((_PrSpriteMem->bShaking && !_PrSpriteMem->bShakingVertical) ? _PrSpriteMem->fShakingForce : 0.f),
			_lby + ((_PrSpriteMem->bShaking && _PrSpriteMem->bShakingVertical) ? _PrSpriteMem->fShakingForce : 0.f),
			_rgba[0],
			_rgba[1],
			_rgba[2],
			_Node->fAlpha,
			_lbtx,
			_lbty,
			_rbx + ((_PrSpriteMem->bShaking && !_PrSpriteMem->bShakingVertical) ? _PrSpriteMem->fShakingForce : 0.f),
			_rby + ((_PrSpriteMem->bShaking && _PrSpriteMem->bShakingVertical) ? _PrSpriteMem->fShakingForce : 0.f),
			_rgba[0],
			_rgba[1],
			_rgba[2],
			_Node->fAlpha,
			_rbtx,
			_rbty
		};
		blGeometryBufferUpdate(_Node->nGBO, 0, (const BLU8*)_vbo, sizeof(_vbo), 0, NULL, 0);
		if (_Node->pEmitParam)
			blDraw(_PrSpriteMem->nSpriteInstTech, _Node->nGBO, _Node->pEmitParam->nCurAlive);
		else if (_Node->nBrightness > 0)
			blDraw(_PrSpriteMem->nSpriteGlowTech, _Node->nGBO, 1);
		else if (_Node->nStrokePixel > 0)
			blDraw(_PrSpriteMem->nSpriteStrokeTech, _Node->nGBO, 1);
		else
			blDraw(_PrSpriteMem->nSpriteTech, _Node->nGBO, 1);
	}
    else
        blDraw(_PrSpriteMem->nSpriteTech, _Node->nGBO, 1);
    for (BLU32 _idx = 0; _idx < _Node->nChildren; ++_idx)
	{
		_BLSpriteNode* _chnode = _Node->pChildren[_idx];
		BLF32 _mat[6], _rmat[6];
		BLF32 _cos = cosf(_chnode->fRotate);
		BLF32 _sin = sinf(_chnode->fRotate);
		BLF32 _pivotx = (_chnode->sPivot.fX - 0.5f) * _chnode->sSize.fX;
		BLF32 _pivoty = (_chnode->sPivot.fY - 0.5f) * _chnode->sSize.fY;
		_mat[0] = (_chnode->fScaleX * _cos);
		_mat[1] = (_chnode->fScaleX * _sin);
		_mat[2] = (-_chnode->fScaleY * _sin);
		_mat[3] = (_chnode->fScaleY * _cos);
		_mat[4] = ((-_pivotx * _chnode->fScaleX) * _cos) + ((_pivoty * _chnode->fScaleY) * _sin) + _chnode->sPos.fX;
		_mat[5] = ((-_pivotx * _chnode->fScaleX) * _sin) + ((-_pivoty * _chnode->fScaleY) * _cos) + _chnode->sPos.fY;
		_rmat[0] = (_mat[0] * _Mat[0]) + (_mat[1] * _Mat[2]);
		_rmat[1] = (_mat[0] * _Mat[1]) + (_mat[1] * _Mat[3]);
		_rmat[2] = (_mat[2] * _Mat[0]) + (_mat[3] * _Mat[2]);
		_rmat[3] = (_mat[2] * _Mat[1]) + (_mat[3] * _Mat[3]);
		_rmat[4] = (_mat[4] * _Mat[0]) + (_mat[5] * _Mat[2]) + _Mat[4];
		_rmat[5] = (_mat[4] * _Mat[1]) + (_mat[5] * _Mat[3]) + _Mat[5];
		_SpriteDraw(_Delta, _chnode, _rmat);
	}
}
BLVoid
_SpriteInit()
{
    _PrSpriteMem = (_BLSpriteMember*)malloc(sizeof(_BLSpriteMember));
    _PrSpriteMem->pNodeList = NULL;
    _PrSpriteMem->nNodeNum = 0;
    _PrSpriteMem->nNodeCap = 0;
	_PrSpriteMem->pCursor = NULL;
    _PrSpriteMem->bShaking = FALSE;
	for (BLU32 _idx = 0; _idx < 8; ++_idx)
		_PrSpriteMem->pTileArray[_idx] = blGenArray(FALSE);
    _PrSpriteMem->nSpriteTech = blGenTechnique("2D.bsl", NULL, FALSE, FALSE);
    _PrSpriteMem->nSpriteInstTech = blGenTechnique("2DInstance.bsl", NULL, FALSE, FALSE);
	_PrSpriteMem->nSpriteStrokeTech = blGenTechnique("2DStroke.bsl", NULL, FALSE, FALSE);
    _PrSpriteMem->nSpriteGlowTech = blGenTechnique("2DGlow.bsl", NULL, FALSE, FALSE);
    blSubscribeEvent(BL_ET_MOUSE, _MouseSubscriber);
	BLU32 _w, _h;
	blUIQueryResolution(&_w, &_h);
	_PrSpriteMem->sViewport.sLT.fX = _PrSpriteMem->sViewport.sLT.fY = 0.f;
	_PrSpriteMem->sViewport.sRB.fX = (BLF32)_w;
	_PrSpriteMem->sViewport.sRB.fY = (BLF32)_h;
}
BLVoid
_SpriteStep(BLU32 _Delta, BLBool _Cursor)
{
    BLU32 _w, _h;
    blUIQueryResolution(&_w, &_h);
    BLF32 _screensz[] = { 2.f / (BLF32)_w, 2.f / (BLF32)_h };
    blTechUniform(_PrSpriteMem->nSpriteTech, BL_UB_F32X2, "ScreenDim", _screensz, sizeof(_screensz));
    blTechUniform(_PrSpriteMem->nSpriteInstTech, BL_UB_F32X2, "ScreenDim", _screensz, sizeof(_screensz));
	blTechUniform(_PrSpriteMem->nSpriteStrokeTech, BL_UB_F32X2, "ScreenDim", _screensz, sizeof(_screensz));
	blTechUniform(_PrSpriteMem->nSpriteGlowTech, BL_UB_F32X2, "ScreenDim", _screensz, sizeof(_screensz));
	BLU8 _blendfactor[4] = { 0 };
	blBlendState(FALSE, TRUE, BL_BF_SRCALPHA, BL_BF_INVSRCALPHA, BL_BF_INVDESTALPHA, BL_BF_ONE, BL_BO_ADD, BL_BO_ADD, _blendfactor);
    if (_Cursor)
    {
        if (!_PrSpriteMem->pCursor)
            return;
        if (!_PrSpriteMem->pCursor->bValid)
            return;
		BLF32 _mat[6];
		BLF32 _pivotx = (_PrSpriteMem->pCursor->sPivot.fX - 0.5f) * _PrSpriteMem->pCursor->sSize.fX;
		BLF32 _pivoty = (_PrSpriteMem->pCursor->sPivot.fY - 0.5f) * _PrSpriteMem->pCursor->sSize.fY;
		_mat[0] = _PrSpriteMem->pCursor->fScaleX;
		_mat[1] = 0;
		_mat[2] = 0;
		_mat[3] = _PrSpriteMem->pCursor->fScaleY;
		_mat[4] = (-_pivotx * _PrSpriteMem->pCursor->fScaleX) + _PrSpriteMem->pCursor->sPos.fX;
		_mat[5] = (-_pivoty * _PrSpriteMem->pCursor->fScaleY) + _PrSpriteMem->pCursor->sPos.fY;
		_SpriteDraw(_Delta, _PrSpriteMem->pCursor, _mat);
    }
    else
    {
        BLRect _scalevp = _PrSpriteMem->sViewport;
        _scalevp.sLT.fX -= (_PrSpriteMem->sViewport.sRB.fX - _PrSpriteMem->sViewport.sLT.fX) * 0.25f;
        _scalevp.sRB.fX += (_PrSpriteMem->sViewport.sRB.fX - _PrSpriteMem->sViewport.sLT.fX) * 0.25f;
        _scalevp.sLT.fY -= (_PrSpriteMem->sViewport.sRB.fY - _PrSpriteMem->sViewport.sLT.fY) * 0.25f;
        _scalevp.sRB.fY += (_PrSpriteMem->sViewport.sRB.fY - _PrSpriteMem->sViewport.sLT.fY) * 0.25f;
		_BLTileInfo* _first = blArrayFrontElement(_PrSpriteMem->pTileArray[0]);
		BLU32 _totalnum = (BLU32)((_scalevp.sRB.fX - _scalevp.sLT.fX) * (_scalevp.sRB.fY - _scalevp.sLT.fY) / (_first->sSize.fX * _first->sSize.fY));
		BLU8* _geomeme = (BLU8*)alloca(_totalnum * 48 * sizeof(BLF32));
		BLGuid _layertex;
		blRasterState(BL_CM_CW, 0, 0.f, TRUE, 0, 0, _w, _h);
		for (BLU32 _idx = 0; _idx < 8; ++_idx)
        {
			BLU32 _tilenum = 0;
            FOREACH_ARRAY (_BLTileInfo*, _tile, _PrSpriteMem->pTileArray[_idx])
            {
                if (blRectContains(&_scalevp, &_tile->sPos))
                {
					_layertex = blGainTexture(blHashUtf8((const BLUtf8*)_tile->aFilename));
					if (_layertex == INVALID_GUID)
					{
						BLGuid _stream;
						if (_tile->aArchive[0] != 0)
							_stream = blGenStream(_tile->aFilename, _tile->aArchive);
						else
						{
							BLAnsi _tmpname[260];
							strcpy(_tmpname, _tile->aFilename);
							BLAnsi _path[260] = { 0 };
							strcpy(_path, blWorkingDir(TRUE));
							strcat(_path, _tmpname);
							_stream = blGenStream(_path, NULL);
							if (INVALID_GUID == _stream)
							{
								memset(_path, 0, sizeof(_path));
								strcpy(_path, blUserFolderDir());
								_stream = blGenStream(_path, NULL);
							}
							if (INVALID_GUID == _stream)
								continue;
						}
						BLU8 _identifier[12];
						blStreamRead(_stream, sizeof(_identifier), _identifier);
						if (_identifier[0] != 0xDD ||
							_identifier[1] != 0xDD ||
							_identifier[2] != 0xDD ||
							_identifier[3] != 0xEE ||
							_identifier[4] != 0xEE ||
							_identifier[5] != 0xEE ||
							_identifier[6] != 0xAA ||
							_identifier[7] != 0xAA ||
							_identifier[8] != 0xAA ||
							_identifier[9] != 0xDD ||
							_identifier[10] != 0xDD ||
							_identifier[11] != 0xDD)
						{
							blDeleteStream(_stream);
							continue;
						}
						BLU32 _width, _height, _depth;
						blStreamRead(_stream, sizeof(BLU32), &_width);
						blStreamRead(_stream, sizeof(BLU32), &_height);
						blStreamRead(_stream, sizeof(BLU32), &_depth);
						BLU32 _array, _faces, _mipmap;
						blStreamRead(_stream, sizeof(BLU32), &_array);
						blStreamRead(_stream, sizeof(BLU32), &_faces);
						blStreamRead(_stream, sizeof(BLU32), &_mipmap);
						BLU32 _fourcc, _channels, _offset;
						blStreamRead(_stream, sizeof(BLU32), &_fourcc);
						blStreamRead(_stream, sizeof(BLU32), &_channels);
						blStreamRead(_stream, sizeof(BLU32), &_offset);
						BLEnum _type = BL_TT_2D;
						blStreamSeek(_stream, _offset);
						BLU32 _imagesz;
						BLEnum _format;
						switch (_fourcc)
						{
						case FOURCC_INTERNAL('B', 'M', 'G', 'T'):
							blStreamRead(_stream, sizeof(BLU32), &_imagesz);
							_format = (_channels == 4) ? BL_TF_RGBA8 : BL_TF_RGB8;
							break;
						case FOURCC_INTERNAL('S', '3', 'T', '1'):
							_imagesz = ((_width + 3) / 4) * ((_height + 3) / 4) * 8;
							_format = BL_TF_BC1;
							break;
						case FOURCC_INTERNAL('S', '3', 'T', '2'):
							_imagesz = ((_width + 3) / 4) * ((_height + 3) / 4) * 8;
							_format = BL_TF_BC1A1;
							break;
						case FOURCC_INTERNAL('S', '3', 'T', '3'):
							_imagesz = ((_width + 3) / 4) * ((_height + 3) / 4) * 16;
							_format = BL_TF_BC3;
							break;
						case FOURCC_INTERNAL('A', 'S', 'T', '1'):
							_imagesz = ((_width + 3) / 4) * ((_height + 3) / 4) * 16;
							_format = BL_TF_ASTC;
							break;
						case FOURCC_INTERNAL('A', 'S', 'T', '2'):
							_imagesz = ((_width + 3) / 4) * ((_height + 3) / 4) * 16;
							_format = BL_TF_ASTC;
							break;
						case FOURCC_INTERNAL('A', 'S', 'T', '3'):
							_imagesz = ((_width + 3) / 4) * ((_height + 3) / 4) * 16;
							_format = BL_TF_ASTC;
							break;
						case FOURCC_INTERNAL('E', 'T', 'C', '1'):
							_imagesz = ((_width + 3) / 4) * ((_height + 3) / 4) * 8;
							_format = BL_TF_ETC2;
							break;
						case FOURCC_INTERNAL('E', 'T', 'C', '2'):
							_imagesz = ((_width + 3) / 4) * ((_height + 3) / 4) * 8;
							_format = BL_TF_ETC2A1;
							break;
						case FOURCC_INTERNAL('E', 'T', 'C', '3'):
							_imagesz = ((_width + 3) / 4) * ((_height + 3) / 4) * 16;
							_format = BL_TF_ETC2A;
							break;
						default:assert(0); break;
						}
						BLU8* _texdata;
						if (_fourcc == FOURCC_INTERNAL('B', 'M', 'G', 'T'))
						{
							BLU8* _data = (BLU8*)malloc(_imagesz);
							blStreamRead(_stream, _imagesz, _data);
							BLU8* _data2 = (BLU8*)malloc(_width * _height * _channels);
							blRLEDecode(_data, _width * _height * _channels, _data2);
							free(_data);
							_texdata = _data2;
						}
						else
						{
							BLU8* _data = (BLU8*)malloc(_imagesz);
							blStreamRead(_stream, _imagesz, _data);
							_texdata = _data;
						}
						_layertex = blGenTexture(blHashUtf8(_tile->aFilename), BL_TT_2D, _format, FALSE, TRUE, FALSE, 1, 1, _width, _height, 1, _texdata);
						free(_texdata);
						blDeleteStream(_stream);
					}
					_tile->nTex = _layertex;
					BLF32 _vbo[] = {
						-_tile->sSize.fX * 0.5f + _tile->sPos.fX,
						-_tile->sSize.fY * 0.5f + _tile->sPos.fY,
						1.f,
						1.f,
						1.f,
						1.f,
						_tile->fTexLTx,
						_tile->fTexLTy,
						_tile->sSize.fX * 0.5f + _tile->sPos.fX,
						-_tile->sSize.fY * 0.5f + _tile->sPos.fY,
						1.f,
						1.f,
						1.f,
						1.f,
						_tile->fTexRBx,
						_tile->fTexLTy,
						-_tile->sSize.fX * 0.5f + _tile->sPos.fX,
						_tile->sSize.fY * 0.5f + _tile->sPos.fY,
						1.f,
						1.f,
						1.f,
						1.f,
						_tile->fTexLTx,
						_tile->fTexRBy,
						-_tile->sSize.fX * 0.5f + _tile->sPos.fX,
						_tile->sSize.fY * 0.5f + _tile->sPos.fY,
						1.f,
						1.f,
						1.f,
						1.f,
						_tile->fTexLTx,
						_tile->fTexRBy,
						_tile->sSize.fX * 0.5f + _tile->sPos.fX,
						-_tile->sSize.fY * 0.5f + _tile->sPos.fY,
						1.f,
						1.f,
						1.f,
						1.f,
						_tile->fTexRBx,
						_tile->fTexLTy,
						_tile->sSize.fX * 0.5f + _tile->sPos.fX,
						_tile->sSize.fY * 0.5f + _tile->sPos.fY,
						1.f,
						1.f,
						1.f,
						1.f,
						_tile->fTexRBx,
						_tile->fTexRBy
					};
					memcpy(_geomeme + _tilenum * 48 * sizeof(BLF32), _vbo, sizeof(_vbo));
					++_tilenum;
                }
            }
			if (_tilenum)
			{
				blTechSampler(_PrSpriteMem->nSpriteTech, "Texture0", _layertex, 0);
				BLEnum _semantic[] = { BL_SL_POSITION, BL_SL_COLOR0, BL_SL_TEXCOORD0 };
				BLEnum _decls[] = { BL_VD_FLOATX2, BL_VD_FLOATX4, BL_VD_FLOATX2 };
				BLAnsi _buf[32] = { 0 };
				sprintf(_buf, "@#tilelayer_%d#@", _idx);
				BLGuid _geo = blGenGeometryBuffer(blHashUtf8((const BLUtf8*)_buf), BL_PT_TRIANGLES, TRUE, _semantic, _decls, 3, _geomeme, _tilenum * 48 * sizeof(BLF32), NULL, 0, BL_IF_INVALID);
				blDraw(_PrSpriteMem->nSpriteTech, _geo, 1);
				blDeleteGeometryBuffer(_geo);
			}
        }
		_SpriteUpdate(_Delta);
        for (BLU32 _idx = 0; _idx < _PrSpriteMem->nNodeNum; ++_idx)
        {
            _BLSpriteNode* _node = _PrSpriteMem->pNodeList[_idx];
            BLF32 _mat[6];
            BLF32 _cos = cosf(_node->fRotate);
            BLF32 _sin = sinf(_node->fRotate);
            BLF32 _pivotx = (_node->sPivot.fX - 0.5f) * _node->sSize.fX;
            BLF32 _pivoty = (_node->sPivot.fY - 0.5f) * _node->sSize.fY;
            _mat[0] = (_node->fScaleX * _cos);
            _mat[1] = (_node->fScaleX * _sin);
            _mat[2] = (-_node->fScaleY * _sin);
            _mat[3] = (_node->fScaleY * _cos);
            _mat[4] = ((-_pivotx * _node->fScaleX) * _cos) + ((_pivoty * _node->fScaleY) * _sin) + _node->sPos.fX;
            _mat[5] = ((-_pivotx * _node->fScaleX) * _sin) + ((-_pivoty * _node->fScaleY) * _cos) + _node->sPos.fY;
            _SpriteDraw(_Delta, _node, _mat);
        }
    }
}
BLVoid
_SpriteDestroy()
{
    blUnsubscribeEvent(BL_ET_MOUSE, _MouseSubscriber);
	for (BLU32 _idx = 0; _idx < 8; ++_idx)
	{		
		FOREACH_ARRAY (_BLTileInfo*, _tileiter, _PrSpriteMem->pTileArray[_idx])
		{
			blDeleteTexture(_tileiter->nTex);
			blDeleteGuid(_tileiter->nID);
			free(_tileiter);
		}
		blDeleteArray(_PrSpriteMem->pTileArray[_idx]);
	}
    if (_PrSpriteMem->pNodeList)
        free(_PrSpriteMem->pNodeList);
    blDeleteTechnique(_PrSpriteMem->nSpriteTech);
    blDeleteTechnique(_PrSpriteMem->nSpriteInstTech);
	blDeleteTechnique(_PrSpriteMem->nSpriteStrokeTech);
	blDeleteTechnique(_PrSpriteMem->nSpriteGlowTech);
    free(_PrSpriteMem);
}
BLGuid
blGenSprite(IN BLAnsi* _Filename, IN BLAnsi* _Archive, IN BLAnsi* _Tag, IN BLF32 _Width, IN BLF32 _Height, IN BLF32 _Zv, IN BLU32 _FPS, IN BLBool _AsTile)
{
	if (_AsTile)
	{
		_BLTileInfo* _tile = (_BLTileInfo*)malloc(sizeof(_BLTileInfo));
		_tile->nTex = INVALID_GUID;
		_tile->sSize.fX = _Width;
		_tile->sSize.fY = _Height;
		_tile->sPos.fX = _tile->sPos.fY = 0.f;
		_tile->bShow = TRUE;
		memset(_tile->aFilename, 0, sizeof(_tile->aFilename));
		memset(_tile->aArchive, 0, sizeof(_tile->aArchive));
		strcpy(_tile->aFilename, _Filename);
		if (_Archive)
			strcpy(_tile->aArchive, _Archive);
		_tile->nID = blGenGuid(_tile, blHashUtf8((const BLUtf8*)_Filename));
		blArrayPushBack(_PrSpriteMem->pTileArray[strtol(_Tag, NULL, 10)], _tile);
		return _tile->nID;
	}
	else
	{
		_BLSpriteNode* _node = (_BLSpriteNode*)malloc(sizeof(_BLSpriteNode));
		_node->nTex = INVALID_GUID;
		_node->nGBO = INVALID_GUID;
		_node->sSize.fX = _Width;
		_node->sSize.fY = _Height;
		_node->sPos.fX = _node->sPos.fY = 0.f;
		_node->sPivot.fX = _node->sPivot.fY = 0.5f;
		_node->sScissor.sLT.fX = _node->sScissor.sLT.fY = -1.f;
		_node->sScissor.sRB.fX = _node->sScissor.sRB.fY = -1.f;
		_node->pParent = NULL;
		_node->pChildren = NULL;
		_node->pAction = NULL;
		_node->pCurAction = NULL;
		_node->pEmitParam = NULL;
		_node->nFPS = _FPS;
		_node->nChildren = 0;
		_node->fRotate = 0.f;
		_node->fScaleX = 1.f;
		_node->fScaleY = 1.f;
		_node->fAlpha = 1.f;
		_node->fZValue = _Zv;
		_node->nDyeColor = 0xFFFFFFFF;
		_node->bDye = FALSE;
		_node->nStrokeColor = 0xFFFFFFFF;
		_node->nStrokePixel = 0;
		_node->nGlowColor = 0xFFFFFFFF;
		_node->nBrightness = 0;
		_node->bShow = TRUE;
		_node->bValid = FALSE;
		_node->nTimePassed = 0;
		_node->nCurFrame = 0;
		_node->nFrameNum = 1;
		memset(_node->aTag, 0, sizeof(_node->aTag));
		memset(_node->aFilename, 0, sizeof(_node->aFilename));
		memset(_node->aArchive, 0, sizeof(_node->aArchive));
		strcpy(_node->aFilename, _Filename);
		if (_Archive)
			strcpy(_node->aArchive, _Archive);
		if (_Tag)
		{
			BLAnsi* _tmp;
			BLAnsi* _tag = (BLAnsi*)alloca(strlen(_Tag) + 1);
			memset(_tag, 0, strlen(_Tag) + 1);
			strcpy(_tag, _Tag);
			_node->nFrameNum = 0;
			_tmp = strtok((BLAnsi*)_tag, ",");
			while (_tmp)
			{
				_node->aTag[_node->nFrameNum] = blHashUtf8(_tmp);
				_tmp = strtok(NULL, ",");
				_node->nFrameNum++;
				if (_node->nFrameNum >= 63)
					break;
			}
		}
		_node->nID = blGenGuid(_node, blHashUtf8((const BLUtf8*)_Filename));
		_FetchResource(_Filename, _Archive, (BLVoid**)&_node, _node->nID, _LoadSprite, _SpriteSetup, TRUE);
		return _node->nID;
	}
}
BLVoid
blDeleteSprite(IN BLGuid _ID)
{
    if (_ID == INVALID_GUID)
        return;
    _BLSpriteNode* _node = (_BLSpriteNode*)blGuidAsPointer(_ID);
    if (!_node)
        return;
    while (_node->nChildren > 0)
        blDeleteSprite(_node->pChildren[0]->nID);
    _DiscardResource(_ID, _UnloadSprite, _SpriteRelease);
	free(_node);
    blDeleteGuid(_ID);
}
BLVoid
blSpriteClear(IN BLBool _Tiles, IN BLBool _Actors)
{
    if (_Tiles)
    {
		for (BLU32 _idx = 0; _idx < 8; ++_idx)
		{
			FOREACH_ARRAY(_BLTileInfo*, _tileiter, _PrSpriteMem->pTileArray[_idx])
			{
				blDeleteTexture(_tileiter->nTex);
				blDeleteGuid(_tileiter->nID);
				free(_tileiter);
			}
			blClearArray(_PrSpriteMem->pTileArray[_idx]);
		}
    }
    if (_Actors)
    {
        for (BLU32 _idx = 0; _idx < _PrSpriteMem->nNodeNum; ++_idx)
        {
            blDeleteSprite(_PrSpriteMem->pNodeList[_idx]->nID);
        }
        free(_PrSpriteMem->pNodeList);
        _PrSpriteMem->nNodeNum = 0;
    }
}
BLBool
blSpriteAttach(IN BLGuid _Parent, IN BLGuid _Child, IN BLBool _AttrInherit)
{
    if (_Parent == INVALID_GUID || _Child == INVALID_GUID)
        return FALSE;
    _BLSpriteNode* _parent = (_BLSpriteNode*)blGuidAsPointer(_Parent);
    _BLSpriteNode* _child = (_BLSpriteNode*)blGuidAsPointer(_Child);
    if (!_parent || !_child)
        return FALSE;
    if (_Parent == _Child)
        return FALSE;
    for (BLU32 _idx = 0 ; _idx < _parent->nChildren ; ++_idx)
    {
        if (_parent->pChildren[_idx] == _child)
            return FALSE;
    }
    if (_child->pParent)
    {
        for (BLU32 _idx = 0 ; _idx < _child->pParent->nChildren ; ++_idx)
        {
            if (_child->pParent->pChildren[_idx] == _child)
            {
                memmove(_child->pParent->pChildren + _idx, _child->pParent->pChildren + _idx + 1, (_child->pParent->nChildren - _idx - 1) * sizeof(_BLSpriteNode*));
                _child->pParent->nChildren--;
                _child->pParent = NULL;
                break;
            }
        }
    }
    else
        _RemoveFromNodeList(_child);
    _child->pParent = _parent;
    _child->fZValue = _AttrInherit ? _parent->fZValue : _child->fZValue;
    _parent->pChildren = (_BLSpriteNode**)realloc(_parent->pChildren, (_parent->nChildren + 1) * sizeof(_BLSpriteNode*));
    BLU32 _idx = 0;
    for (; _idx < _parent->nChildren; ++_idx)
    {
        if (_parent->pChildren[_idx]->fZValue >= _child->fZValue)
            break;
    }
    memmove(_parent->pChildren + _idx + 1, _parent->pChildren + _idx, (_parent->nChildren - _idx) * sizeof(_BLSpriteNode*));
    _parent->pChildren[_idx] = _child;
    _parent->nChildren++;
    blSpriteScissor(_Child, _parent->sScissor.sLT.fX, _parent->sScissor.sLT.fY, _parent->sScissor.sRB.fX - _parent->sScissor.sLT.fX, _parent->sScissor.sRB.fY - _parent->sScissor.sLT.fY);
    if (_AttrInherit)
    {
        blSpriteDyeing(_Child, _parent->nDyeColor, _parent->bDye, TRUE);
        blSpriteVisibility(_Child, _parent->bShow, TRUE);
        blSpriteAlpha(_Child, _parent->fAlpha, TRUE);
        blSpriteGlow(_Child, _parent->nGlowColor, _parent->nBrightness, TRUE);
        blSpriteStroke(_Child, _parent->nStrokeColor, _parent->nStrokePixel, TRUE);
		blSpritePivot(_Child, _parent->sPivot.fX, _parent->sPivot.fY, TRUE);
        blSpriteZValue(_Child, _parent->fZValue, TRUE);
    }
    return TRUE;
}
BLBool
blSpriteDetach(IN BLGuid _Parent, IN BLGuid _Child)
{
    if (_Parent == INVALID_GUID || _Child == INVALID_GUID)
        return FALSE;
    _BLSpriteNode* _parent = (_BLSpriteNode*)blGuidAsPointer(_Parent);
    _BLSpriteNode* _child = (_BLSpriteNode*)blGuidAsPointer(_Child);
    if (!_parent || !_child)
        return FALSE;
    if (!_child->pParent)
        return FALSE;
    if (_child->pParent->nID != _Parent || _Parent == _Child)
        return FALSE;
    BLBool _found = FALSE;
    for (BLU32 _idx = 0 ; _idx < _parent->nChildren ; ++_idx)
    {
        if (_parent->pChildren[_idx] == _child)
        {
            memmove(_parent->pChildren + _idx, _parent->pChildren + _idx + 1, (_parent->nChildren - _idx - 1) * sizeof(_BLSpriteNode*));
            _found = TRUE;
            break;
        }
    }
    if (!_found)
        return FALSE;
    _parent->nChildren--;
    _child->pParent = NULL;
    _AddToNodeList(_child);
    return TRUE;
}
BLBool
blSpriteQuery(IN BLGuid _ID, OUT BLF32* _Width, OUT BLF32* _Height,OUT BLF32* _XPos, OUT BLF32* _YPos, OUT BLF32* _Zv, OUT BLF32* _Rotate, OUT BLF32* _XScale, OUT BLF32* _YScale, OUT BLF32* _Alpha, OUT BLBool* _Show)
{
    if (_ID == INVALID_GUID)
        return FALSE;
    _BLSpriteNode* _node = (_BLSpriteNode*)blGuidAsPointer(_ID);
    if (!_node)
        return FALSE;
    *_Width = _node->sSize.fX;
    *_Height = _node->sSize.fY;
    *_XPos = _node->sPos.fX;
    *_YPos = _node->sPos.fY;
    *_Zv = _node->fZValue;
    *_Rotate = _node->fRotate;
    *_XScale = _node->fScaleX;
    *_YScale = _node->fScaleY;
    *_Alpha = _node->fAlpha;
    *_Show = _node->bShow;
    return TRUE;
}
BLBool
blSpriteVisibility(IN BLGuid _ID, IN BLBool _Show, IN BLBool _Passdown)
{
    if (_ID == INVALID_GUID)
        return FALSE;
    _BLSpriteNode* _node = (_BLSpriteNode*)blGuidAsPointer(_ID);
    if (!_node)
        return FALSE;
    _node->bShow = _Show;
    if (_Passdown)
    {
        for (BLU32 _idx = 0; _idx < _node->nChildren ; ++_idx)
            blSpriteVisibility(_node->pChildren[_idx]->nID, _Show, _Passdown);
    }
    return TRUE;
}
BLBool 
blSpriteFlip(IN BLGuid _ID, IN BLBool _FlipX, IN BLBool _FlipY, IN BLBool _Passdown)
{
	if (_ID == INVALID_GUID)
		return FALSE;
	_BLSpriteNode* _node = (_BLSpriteNode*)blGuidAsPointer(_ID);
	if (!_node)
		return FALSE;
	_node->bFlipX = _FlipX;
	_node->bFlipY = _FlipY;
	if (_Passdown)
	{
		for (BLU32 _idx = 0; _idx < _node->nChildren; ++_idx)
			blSpriteFlip(_node->pChildren[_idx]->nID, _FlipX, _FlipY, _Passdown);
	}
	return TRUE;
}
BLBool
blSpritePivot(IN BLGuid _ID, IN BLF32 _PivotX, IN BLF32 _PivotY, IN BLBool _Passdown)
{
    if (_ID == INVALID_GUID)
        return FALSE;
    _BLSpriteNode* _node = (_BLSpriteNode*)blGuidAsPointer(_ID);
    if (!_node)
        return FALSE;
    _node->sPivot.fX = blScalarClamp(_PivotX, 0.f, 1.f);
    _node->sPivot.fY = blScalarClamp(_PivotY, 0.f, 1.f);
    if (_Passdown)
    {
        for (BLU32 _idx = 0; _idx < _node->nChildren ; ++_idx)
			blSpritePivot(_node->pChildren[_idx]->nID, _PivotX, _PivotY, _Passdown);
    }
    return TRUE;
}
BLBool
blSpriteAlpha(IN BLGuid _ID, IN BLF32 _Alpha, IN BLBool _Passdown)
{
    if (_ID == INVALID_GUID)
        return FALSE;
    _BLSpriteNode* _node = (_BLSpriteNode*)blGuidAsPointer(_ID);
    if (!_node)
        return FALSE;
    _node->fAlpha = _Alpha;
    if (_Passdown)
    {
        for (BLU32 _idx = 0; _idx < _node->nChildren ; ++_idx)
            blSpriteAlpha(_node->pChildren[_idx]->nID, _Alpha, _Passdown);
    }
    return TRUE;
}
BLBool
blSpriteZValue(IN BLGuid _ID, IN BLF32 _Zv, IN BLBool _Passdown)
{
    if (_ID == INVALID_GUID)
        return FALSE;
    _BLSpriteNode* _node = (_BLSpriteNode*)blGuidAsPointer(_ID);
    if (!_node)
        return FALSE;
    if (!_node->pParent)
    {
        _RemoveFromNodeList(_node);
        _node->fZValue = _Zv;
        _AddToNodeList(_node);
    }
    else
    {
        BLBool _append = FALSE;
        BLU32 _idx = 0;
        for (; _idx < _node->pParent->nChildren ; ++_idx)
        {
            if (_node->pParent->pChildren[_idx] == _node)
            {
                memmove(_node->pParent->pChildren + _idx, _node->pParent->pChildren + _idx + 1, (_node->pParent->nChildren - _idx - 1) * sizeof(_BLSpriteNode*));
                _node->pParent->nChildren--;
                _append = TRUE;
                break;
            }
        }
        _idx = 0;
        _node->fZValue = _Zv;
        for (; _idx < _node->pParent->nChildren; ++_idx)
        {
            if (_node->pParent->pChildren[_idx]->fZValue >= _node->fZValue)
            {
                memmove(_node->pParent->pChildren + _idx + 1, _node->pParent->pChildren + _idx, (_node->pParent->nChildren - _idx) * sizeof(_BLSpriteNode*));
                _node->pParent->pChildren[_idx] = _node;
                _node->pParent->nChildren++;
                _append = FALSE;
                break;
            }
        }
        if (_append)
        {
            _node->pParent->pChildren[_node->pParent->nChildren] = _node;
            _node->pParent->nChildren++;
        }
    }
    if (_Passdown)
    {
        _BLSpriteNode** _tmp = (_BLSpriteNode**)alloca(_node->nChildren * sizeof(_BLSpriteNode*));
        memcpy(_tmp, _node->pChildren, _node->nChildren * sizeof(_BLSpriteNode*));
        for (BLU32 _idx = 0; _idx < _node->nChildren ; ++_idx)
            blSpriteZValue(_tmp[_idx]->nID, _Zv, _Passdown);
    }
    return TRUE;
}
BLBool
blSpriteStroke(IN BLGuid _ID, IN BLU32 _Color, IN BLU32 _Pixel, IN BLBool _Passdown)
{
    if (_ID == INVALID_GUID)
        return FALSE;
    _BLSpriteNode* _node = (_BLSpriteNode*)blGuidAsPointer(_ID);
    if (!_node)
        return FALSE;
    _node->nStrokeColor = _Color;
    _node->nStrokePixel = _Pixel;
    if (_Passdown)
    {
        for (BLU32 _idx = 0; _idx < _node->nChildren ; ++_idx)
            blSpriteStroke(_node->pChildren[_idx]->nID, _Color, _Pixel, _Passdown);
    }
    return TRUE;    
}
BLBool
blSpriteGlow(IN BLGuid _ID, IN BLU32 _Color, IN BLU32 _Brightness, IN BLBool _Passdown)
{
    if (_ID == INVALID_GUID)
        return FALSE;
    _BLSpriteNode* _node = (_BLSpriteNode*)blGuidAsPointer(_ID);
    if (!_node)
        return FALSE;
    _node->nGlowColor = _Color;
    _node->nBrightness = _Brightness;
    if (_Passdown)
    {
        for (BLU32 _idx = 0; _idx < _node->nChildren ; ++_idx)
            blSpriteGlow(_node->pChildren[_idx]->nID, _Color, _Brightness, _Passdown);
    }
    return TRUE;
}
BLBool
blSpriteDyeing(IN BLGuid _ID, IN BLU32 _Color, IN BLBool _Dye, IN BLBool _Passdown)
{
    if (_ID == INVALID_GUID)
        return FALSE;
    _BLSpriteNode* _node = (_BLSpriteNode*)blGuidAsPointer(_ID);
    if (!_node)
        return FALSE;
    _node->nDyeColor = _Color;
    _node->bDye = _Dye;
    if (_Passdown)
    {
        for (BLU32 _idx = 0; _idx < _node->nChildren ; ++_idx)
            blSpriteDyeing(_node->pChildren[_idx]->nID, _Color, _Dye, _Passdown);
    }
    return TRUE;
}
BLBool
blSpriteTransformReset(IN BLGuid _ID, IN BLF32 _XPos, IN BLF32 _YPos, IN BLF32 _Rotate, IN BLF32 _ScaleX, IN BLF32 _ScaleY)
{
    if (_ID == INVALID_GUID)
        return FALSE;
    _BLSpriteNode* _node = (_BLSpriteNode*)blGuidAsPointer(_ID);
    if (!_node)
        return FALSE;
    _node->sPos.fX = _XPos;
    _node->sPos.fY = _YPos;
    _node->fRotate = _Rotate;
    _node->fScaleX = _ScaleX;
    _node->fScaleY = _ScaleY;
    return TRUE;
}
BLBool
blSpriteMove(IN BLGuid _ID, IN BLF32 _XVec, IN BLF32 _YVec)
{
    if (_ID == INVALID_GUID)
        return FALSE;
    _BLSpriteNode* _node = (_BLSpriteNode*)blGuidAsPointer(_ID);
    if (!_node)
        return FALSE;
    _node->sPos.fX += _XVec;
    _node->sPos.fY += _YVec;
    return TRUE;
}
BLBool
blSpriteScale(IN BLGuid _ID, IN BLF32 _XScale, IN BLF32 _YScale)
{
    if (_ID == INVALID_GUID)
        return FALSE;
    _BLSpriteNode* _node = (_BLSpriteNode*)blGuidAsPointer(_ID);
    if (!_node)
        return FALSE;
    _node->fScaleX *= _XScale;
    _node->fScaleY *= _YScale;
    return TRUE;
}
BLBool
blSpriteRotate(IN BLGuid _ID, IN BLF32 _Rotate)
{
    if (_ID == INVALID_GUID)
        return FALSE;
    _BLSpriteNode* _node = (_BLSpriteNode*)blGuidAsPointer(_ID);
    if (!_node)
        return FALSE;
    _node->fRotate += _Rotate * PI_INTERNAL / 180.0f;
    return TRUE;
}
BLBool
blSpriteScissor(IN BLGuid _ID, IN BLF32 _XPos, IN BLF32 _YPos, IN BLF32 _Width, IN BLF32 _Height)
{
    if (_ID == INVALID_GUID)
        return FALSE;
    _BLSpriteNode* _node = (_BLSpriteNode*)blGuidAsPointer(_ID);
    if (!_node)
        return FALSE;
    _node->sScissor.sLT.fX = _XPos;
    _node->sScissor.sLT.fY = _YPos;
    _node->sScissor.sRB.fX = _XPos + _Width;
    _node->sScissor.sRB.fY = _YPos + _Height;
    for (BLU32 _idx = 0; _idx < _node->nChildren ; ++_idx)
		blSpriteScissor(_node->pChildren[_idx]->nID, _XPos, _YPos, _Width, _Height);
    return TRUE;
}
BLVoid
blSpriteTile(IN BLGuid _ID, IN BLAnsi* _ImageFile, IN BLAnsi* _Archive, IN BLBool _FlipX, IN BLBool _FlipY, IN BLBool _Diagonal, IN BLF32 _TexLTx, IN BLF32 _TexLTy, IN BLF32 _TexRBx, IN BLF32 _TexRBy, IN BLF32 _PosX, IN BLF32 _PosY)
{
    if (_ID == INVALID_GUID)
        return;
	_BLTileInfo* _tile = (_BLTileInfo*)blGuidAsPointer(_ID);
    if (!_tile)
        return;
    memset(_tile->aFilename, 0, sizeof(_tile->aFilename));
    strcpy(_tile->aFilename, _ImageFile);
    memset(_tile->aArchive, 0, sizeof(_tile->aArchive));
    if (_Archive)
        strcpy(_tile->aArchive, _Archive);
    _tile->bFlipX = _FlipX;
    _tile->bFlipY = _FlipY;
    _tile->fTexLTx = _TexLTx;
    _tile->fTexLTy = _TexLTy;
    _tile->fTexRBx = _TexRBx;
    _tile->fTexRBy = _TexRBy;
	_tile->sPos.fX = _PosX;
	_tile->sPos.fY = _PosY;
}
BLBool
blSpriteAsEmit(IN BLGuid _ID, IN BLF32 _EmitAngle, IN BLF32 _EmitterRadius, IN BLF32 _Life, IN BLU32 _MaxAlive, IN BLU32 _GenPerSec, IN BLF32 _DirectionX, IN BLF32 _DirectionY, IN BLF32 _Velocity, IN BLF32 _VelVariance, IN BLF32 _Rotation, IN BLF32 _RotVariance, IN BLF32 _Scale, IN BLF32 _ScaleVariance, IN BLU32 _FadeColor)
{
    if (_ID == INVALID_GUID)
        return FALSE;
	_BLSpriteNode* _node = (_BLSpriteNode*)blGuidAsPointer(_ID);
	if (!_node)
		return FALSE;
	if (!_node->pEmitParam)
    {
        _node->pEmitParam = (_BLEmitParam*)malloc(sizeof(_BLEmitParam));
        _node->pEmitParam->pAge = (BLU32*)malloc(_MaxAlive * sizeof(BLU32));
        memset(_node->pEmitParam->pAge, 0, _MaxAlive * sizeof(BLU32));
        _node->pEmitParam->pPositionX = (BLF32*)malloc(_MaxAlive * sizeof(BLF32));
        _node->pEmitParam->pPositionY = (BLF32*)malloc(_MaxAlive * sizeof(BLF32));
        _node->pEmitParam->pEmitAngle = (BLF32*)malloc(_MaxAlive * sizeof(BLF32));
        _node->pEmitParam->pVelocity = (BLF32*)malloc(_MaxAlive * sizeof(BLF32));
        _node->pEmitParam->pRotScale = (BLU32*)malloc(_MaxAlive * sizeof(BLU32));
    }
	_node->pEmitParam->fEmitterRadius = _EmitterRadius;
	_node->pEmitParam->nLife = (BLU32)(_Life * 1000);
	_node->pEmitParam->nMaxAlive = _MaxAlive;
	_node->pEmitParam->fGenPerMSec = (BLF32)_GenPerSec / 1000.f;
    BLVec2 _dir;
    _dir.fX = _DirectionX;
    _dir.fY = _DirectionY;
    blVec2Normalize(&_dir);
	_node->pEmitParam->fDirectionX = _dir.fX;
    _node->pEmitParam->fDirectionY = _dir.fY;
    _node->pEmitParam->fVelocity = _Velocity;
    _node->pEmitParam->fVelVariance = _VelVariance;
	_node->pEmitParam->fEmitAngle = _EmitAngle * PI_INTERNAL / 180.f;
    _node->pEmitParam->fRotation = _Rotation * PI_INTERNAL / 180.f;
    _node->pEmitParam->fRotVariance = _RotVariance * PI_INTERNAL / 180.f;
    _node->pEmitParam->fScale = _Scale;
    _node->pEmitParam->fScaleVariance = _ScaleVariance;
    _node->pEmitParam->nFadeColor = _FadeColor;
    _node->pEmitParam->nCurAlive = 0;
    return TRUE;
}
BLBool
blSpriteAsCursor(IN BLGuid _ID)
{
    if (_ID == INVALID_GUID)
        return FALSE;
    _BLSpriteNode* _node = (_BLSpriteNode*)blGuidAsPointer(_ID);
    if (!_node)
        return FALSE;
	_PrSpriteMem->pCursor = _node;
    return TRUE;
}
BLBool
blSpriteActionBegin(IN BLGuid _ID)
{
    if (_ID == INVALID_GUID)
        return FALSE;
    _BLSpriteNode* _node = (_BLSpriteNode*)blGuidAsPointer(_ID);
    if (!_node)
        return FALSE;
    if (_node->pAction)
    {
        _BLSpriteAction* _tmp = _node->pAction;
		while (_tmp)
		{
			_BLSpriteAction* _tmpnext = _tmp->pNext;
			if (_tmp->pNeighbor)
			{
				_BLSpriteAction* _tmpneb = _tmp->pNeighbor;
				while (_tmp)
				{
					if (_tmp->eActionType == SPACTION_MOVE_INTERNAL)
					{
						free(_tmp->uAction.sMove.pXPath);
						free(_tmp->uAction.sMove.pYPath);
					}
					free(_tmp);
					_tmp = _tmpneb;
					_tmpneb = _tmp ? _tmp->pNeighbor : NULL;
				}
			}
			else
			{
				if (_tmp->eActionType == SPACTION_MOVE_INTERNAL)
				{
					free(_tmp->uAction.sMove.pXPath);
					free(_tmp->uAction.sMove.pYPath);
				}
				free(_tmp);
			}
			_tmp = _tmpnext;
		}
    }
	_PrSpriteMem->bParallelEdit = FALSE;
    return TRUE;
}
BLBool
blSpriteActionEnd(IN BLGuid _ID, IN BLBool _Delete)
{
    if (_ID == INVALID_GUID)
        return FALSE;
    _BLSpriteNode* _node = (_BLSpriteNode*)blGuidAsPointer(_ID);
    if (!_node)
        return FALSE;
    if (!_node->pAction)
        return FALSE;
	if (_Delete)
	{
		_BLSpriteAction* _act = (_BLSpriteAction*)malloc(sizeof(_BLSpriteAction));
		_act->pNeighbor = NULL;
		_act->pNext = NULL;
		_act->bParallel = FALSE;
		_act->bLoop = FALSE;
		_act->eActionType = SPACTION_DEAD_INTERNAL;
		_act->fCurTime = 0.f;
		_act->fTotalTime = 0.f;
		_act->uAction.sDead.bDead = TRUE;
		_BLSpriteAction* _lastact = _node->pAction;
		while (_lastact->pNext)
			_lastact = _lastact->pNext;
		_lastact->pNext = _act;
	}
	_node->pCurAction = NULL;
	_PrSpriteMem->bParallelEdit = FALSE;
    return TRUE;
}
BLBool 
blSpriteActionPlay(IN BLGuid _ID)
{
	if (_ID == INVALID_GUID)
		return FALSE;
	_BLSpriteNode* _node = (_BLSpriteNode*)blGuidAsPointer(_ID);
	if (!_node)
		return FALSE;
	if (!_node->pAction)
		return FALSE;
	_node->pCurAction = _node->pAction;
	return TRUE;
}
BLBool 
blSpriteActionStop(IN BLGuid _ID)
{
	if (_ID == INVALID_GUID)
		return FALSE;
	_BLSpriteNode* _node = (_BLSpriteNode*)blGuidAsPointer(_ID);
	if (!_node)
		return FALSE;
	if (!_node->pAction)
		return FALSE;
	_node->pCurAction = NULL;
	return TRUE;
}
BLBool
blSpriteParallelBegin(IN BLGuid _ID)
{
    if (_ID == INVALID_GUID)
        return FALSE;
	if (_PrSpriteMem->bParallelEdit)
		return FALSE;
	_PrSpriteMem->bParallelEdit = TRUE;
    return TRUE;
}
BLBool
blSpriteParallelEnd(IN BLGuid _ID)
{
    if (_ID == INVALID_GUID)
        return FALSE;
	if (!_PrSpriteMem->bParallelEdit)
		return FALSE;
    _PrSpriteMem->bParallelEdit = FALSE;
    _BLSpriteNode* _node = (_BLSpriteNode*)blGuidAsPointer(_ID);
    _BLSpriteAction* _lastact = _node->pAction;
    while (_lastact->pNext)
        _lastact = _lastact->pNext;
    _lastact->bParallel = FALSE;
    return TRUE;
}
BLBool
blSpriteActionMove(IN BLGuid _ID, IN BLF32* _XPath, IN BLF32* _YPath, IN BLU32 _PathNum, IN BLF32 _Acceleration, IN BLF32 _Time, IN BLBool _Loop)
{
    if (_ID == INVALID_GUID)
        return FALSE;
    _BLSpriteNode* _node = (_BLSpriteNode*)blGuidAsPointer(_ID);
    if (!_node)
        return FALSE;
	if (_PathNum <= 0 || _Time <= 0.f)
		return FALSE;
	_BLSpriteAction* _act = (_BLSpriteAction*)malloc(sizeof(_BLSpriteAction));
	_act->pNeighbor = NULL;
	_act->pNext = NULL;
    _act->bLoop = _Loop;
    _act->bParallel = _PrSpriteMem->bParallelEdit;
	_act->eActionType = SPACTION_MOVE_INTERNAL;
	_act->fCurTime = 0.f;
	_act->fTotalTime = _Time;
	_act->uAction.sMove.fAcceleration = _Acceleration;
	BLF32 _s = sqrtf((_XPath[0] - _node->sPos.fX) * (_XPath[0] - _node->sPos.fX) + (_YPath[0] - _node->sPos.fY) * (_YPath[0] - _node->sPos.fY));
	for (BLU32 _idx = 1; _idx < _PathNum; ++_idx)
		_s += sqrtf((_XPath[_idx] - _XPath[_idx - 1]) * (_XPath[_idx] - _XPath[_idx - 1]) + (_YPath[_idx] - _YPath[_idx - 1]) * (_YPath[_idx] - _YPath[_idx - 1]));
	_act->uAction.sMove.fVelocity = _s / _Time - 0.5f * _Acceleration * _Time;
	_act->uAction.sMove.pXPath = (BLF32*)malloc((_PathNum + 1) * sizeof(BLF32));
	_act->uAction.sMove.pYPath = (BLF32*)malloc((_PathNum + 1) * sizeof(BLF32));
	_act->uAction.sMove.pXPath[0] = _node->sPos.fX;
	_act->uAction.sMove.pYPath[0] = _node->sPos.fY;
	_act->uAction.sMove.nPathNum = _PathNum + 1;
	memcpy(_act->uAction.sMove.pXPath + 1, _XPath, _PathNum * sizeof(BLF32));
	memcpy(_act->uAction.sMove.pYPath + 1, _YPath, _PathNum * sizeof(BLF32));
	if (!_node->pAction)
	{
		_node->pAction = _act;
		return TRUE;
	}
	else
	{
		_BLSpriteAction* _lastact = _node->pAction;
		while (_lastact->pNext)
			_lastact = _lastact->pNext;
		if (_PrSpriteMem->bParallelEdit && _lastact->bParallel)
		{
			while (_lastact->pNeighbor)
				_lastact = _lastact->pNeighbor;
			_lastact->pNeighbor = _act;
		}
		else
			_lastact->pNext = _act;
	}
    return TRUE;
}
BLBool
blSpriteActionRotate(IN BLGuid _ID, IN BLF32 _Angle, IN BLBool _ClockWise, IN BLF32 _Time, IN BLBool _Loop)
{
    if (_ID == INVALID_GUID)
        return FALSE;
    _BLSpriteNode* _node = (_BLSpriteNode*)blGuidAsPointer(_ID);
    if (!_node)
        return FALSE;
	_BLSpriteAction* _act = (_BLSpriteAction*)malloc(sizeof(_BLSpriteAction));
	_act->pNeighbor = NULL;
	_act->pNext = NULL;
	_act->bLoop = _Loop;
	_act->bParallel = _PrSpriteMem->bParallelEdit;
	_act->eActionType = SPACTION_ROTATE_INTERNAL;
	_act->fCurTime = 0.f;
	_act->fTotalTime = _Time;
    _act->uAction.sRotate.bClockWise = _ClockWise;
	_act->uAction.sRotate.fAngle = _Angle;
	if (!_node->pAction)
	{
		_node->pAction = _act;
		return TRUE;
	}
	else
	{
		_BLSpriteAction* _lastact = _node->pAction;
		while (_lastact->pNext)
			_lastact = _lastact->pNext;
		if (_PrSpriteMem->bParallelEdit && _lastact->bParallel)
		{
			while (_lastact->pNeighbor)
				_lastact = _lastact->pNeighbor;
			_lastact->pNeighbor = _act;
		}
		else
			_lastact->pNext = _act;
	}
    return TRUE;
}
BLBool
blSpriteActionScale(IN BLGuid _ID, IN BLF32 _XScale, IN BLF32 _YScale, IN BLBool _Reverse, IN BLF32 _Time, IN BLBool _Loop)
{
    if (_ID == INVALID_GUID)
        return FALSE;
    _BLSpriteNode* _node = (_BLSpriteNode*)blGuidAsPointer(_ID);
    if (!_node)
        return FALSE;
	_BLSpriteAction* _act = (_BLSpriteAction*)malloc(sizeof(_BLSpriteAction));
	_act->pNeighbor = NULL;
	_act->pNext = NULL;
	_act->bLoop = _Loop;
	_act->bParallel = _PrSpriteMem->bParallelEdit;
	_act->eActionType = SPACTION_SCALE_INTERNAL;
	_act->fCurTime = 0.f;
	_act->fTotalTime = _Time;
	_act->uAction.sScale.fXInitScale = _node->fScaleX;
	_act->uAction.sScale.fYInitScale = _node->fScaleY;
	_act->uAction.sScale.fXScale = _XScale;
	_act->uAction.sScale.fYScale = _YScale;
    _act->uAction.sScale.bReverse = _Reverse;
	if (!_node->pAction)
	{
		_node->pAction = _act;
		return TRUE;
	}
	else
	{
		_BLSpriteAction* _lastact = _node->pAction;
		while (_lastact->pNext)
			_lastact = _lastact->pNext;
		if (_PrSpriteMem->bParallelEdit && _lastact->bParallel)
		{
			while (_lastact->pNeighbor)
				_lastact = _lastact->pNeighbor;
			_lastact->pNeighbor = _act;
		}
		else
			_lastact->pNext = _act;
	}
    return TRUE;
}
BLBool
blSpriteActionAlpha(IN BLGuid _ID, IN BLF32 _Alpha, IN BLBool _Reverse, IN BLF32 _Time, IN BLBool _Loop)
{
    if (_ID == INVALID_GUID)
        return FALSE;
    _BLSpriteNode* _node = (_BLSpriteNode*)blGuidAsPointer(_ID);
    if (!_node)
        return FALSE;
	_BLSpriteAction* _act = (_BLSpriteAction*)malloc(sizeof(_BLSpriteAction));
	_act->pNeighbor = NULL;
	_act->pNext = NULL;
	_act->bLoop = _Loop;
	_act->bParallel = _PrSpriteMem->bParallelEdit;
	_act->eActionType = SPACTION_ALPHA_INTERNAL;
	_act->fCurTime = 0.f;
	_act->fTotalTime = _Time;
	_act->uAction.sAlpha.fInitAlpha = _node->fAlpha;
	_act->uAction.sAlpha.fAlpha = _Alpha;
	_act->uAction.sAlpha.bReverse = _Reverse;
	if (!_node->pAction)
	{
		_node->pAction = _act;
		return TRUE;
	}
	else
	{
		_BLSpriteAction* _lastact = _node->pAction;
		while (_lastact->pNext)
			_lastact = _lastact->pNext;
		if (_PrSpriteMem->bParallelEdit && _lastact->bParallel)
		{
			while (_lastact->pNeighbor)
				_lastact = _lastact->pNeighbor;
			_lastact->pNeighbor = _act;
		}
		else
			_lastact->pNext = _act;
	}
    return TRUE;
}
BLVoid
blViewportQuery(OUT BLF32* _LTPosX, OUT BLF32* _LTPosY, OUT BLF32* _RBPosX, OUT BLF32* _RBPosY)
{
    *_LTPosX = _PrSpriteMem->sViewport.sLT.fX;
    *_LTPosY = _PrSpriteMem->sViewport.sLT.fY;
    *_RBPosX = _PrSpriteMem->sViewport.sRB.fX;
    *_RBPosY = _PrSpriteMem->sViewport.sRB.fY;
}
BLVoid
blViewportMove(IN BLF32 _XVec, IN BLF32 _YVec)
{
	_PrSpriteMem->sViewport.sLT.fX += _XVec;
	_PrSpriteMem->sViewport.sLT.fY += _YVec;
	_PrSpriteMem->sViewport.sRB.fX += _XVec;
	_PrSpriteMem->sViewport.sRB.fY += _YVec;
}
BLVoid 
blViewportShake(IN BLF32 _Time, IN BLBool _Vertical, IN BLF32 _Force)
{
    _PrSpriteMem->bShaking = TRUE;
    _PrSpriteMem->fShakingTime = _Time;
    _PrSpriteMem->bShakingVertical = _Vertical;
    _PrSpriteMem->fShakingForce = _Force;
}
