#include "SkeltalAnimation.h"
#include <Loader/MeshLoader.h>
#include <Renderer/Mesh/Skeleton.h>
#include <fstream>
#include <algorithm>
#include <iostream>


SkeltalAnimation::SkeltalAnimation(const std::string& fileName) :_maxFrame{ 0 }, _plaingFrame{ 0.0f }, _playRate{ 1.0f }{

	//ファイル名が引数にあればロードする
	if(fileName != ""){
		load(fileName);
	}
}

SkeltalAnimation::~SkeltalAnimation(){
}

void SkeltalAnimation::load(const std::string& fileName){

	//ファイル名から拡張子を除いて固有識別子にする
	int path_i = fileName.find_last_of("/") + 1;
	_name = fileName.substr(path_i);

	path_i = _name.find_last_of(".");
	_name = _name.substr(0, path_i);

	//  ios::in は読み込み専用  ios::binary はバイナリ形式
	std::ifstream fin(fileName, std::ios::in | std::ios::binary);

	if(!fin){
		std::cout << "ファイル " << fileName << " が開けません";
	}

	//ボーンサイズ
	int boneSize = 0;
	fin.read(reinterpret_cast<char*>( &boneSize ), sizeof(int));

	//キーサイズ
	int keySize = 0;
	fin.read(reinterpret_cast<char*>( &keySize ), sizeof(int));

	//最大フレーム数
	fin.read(reinterpret_cast<char*>( &_maxFrame ), sizeof(int));

	//ボーン数だけメモリを確保
	_animationBones.reserve(boneSize);
	_frameCache.reserve(boneSize);
	_frameCache = std::vector<TransformQ>(boneSize);

	//ボーンすべて読み終わるまでループ
	while(_animationBones.size() != boneSize){

		AnimationBone bone;

		//ボーン内のキーフレームを読み終わるまでループ
		while(bone.keys.size() != keySize){

			//キーフレーム構造体をバイナリデータから生成
			TransformQ key;

			fin.read(reinterpret_cast<char*>( &key.position ), sizeof(Vector3));
			fin.read(reinterpret_cast<char*>( &key.rotation ), sizeof(Quaternion));
			fin.read(reinterpret_cast<char*>( &key.scale ), sizeof(Vector3));

			bone.keys.emplace_back(key);
		}

		_animationBones.emplace_back(bone);
	}

	fin.close();
}

void SkeltalAnimation::update(float deltaTime){

	//フレーム更新
	//逆再生には未対応
	_plaingFrame += 0.5f * getPlayRate();

	if(_plaingFrame >= _maxFrame){
		_plaingFrame -= _maxFrame;
	}

	for(int i = 0; i < _animationBones.size(); i++){

		//どのフレーム間か調べる
		int firstFrame = static_cast<int>( std::floorf(_plaingFrame) );
		int secondFrame = static_cast<int>( std::ceilf(_plaingFrame) );

		//フレームの中間値(0.0f~1.0f)
		const float lerpValue = _plaingFrame - firstFrame;

		//最終フレームを参照したら0に戻す
		if(secondFrame == _maxFrame){
			secondFrame = 0;
		}

		//参照するキーを取得
		const TransformQ firstkey = _animationBones[i][firstFrame];
		const TransformQ secondKey = _animationBones[i][secondFrame];

		//中間値で補完した各座標を取得
		const Vector3 lerpPosition = Vector3::lerp(firstkey.position, secondKey.position, lerpValue);
		const Vector3 lerpScale = Vector3::lerp(firstkey.scale, secondKey.scale, lerpValue);
		const Quaternion slerpRotation = Quaternion::slerp(firstkey.rotation, secondKey.rotation, lerpValue);

		//行列に変換
		const Matrix4 translate = Matrix4::translateXYZ(lerpPosition);
		const Matrix4 scale = Matrix4::scaleXYZ(lerpScale);
		const Matrix4 rotation = Matrix4::matrixFromQuaternion(slerpRotation);

		//行列合成
		const Matrix4 matrix = Matrix4::multiply(Matrix4::multiply(scale, rotation), translate);

		//最終アニメーション変換行列を格納
		_animationBones[i].animatedMatrix = matrix;

		//補完したボーン情報をキャッシュする
		const TransformQ transformCache(lerpPosition, slerpRotation, lerpScale);
		_frameCache[i] = transformCache;
	}
}

const Matrix4& SkeltalAnimation::getAnimationMatrix(int boneIndex) const{
	return _animationBones[boneIndex].animatedMatrix;
}

void SkeltalAnimation::resetAnimation(){
	_plaingFrame = 0;
}

void SkeltalAnimation::setPlayRate(float rate){
	_playRate = rate;
}

float SkeltalAnimation::getPlayRate() const{
	return _playRate;
}

int SkeltalAnimation::getMaxFrame() const{
	return _maxFrame;
}

std::string SkeltalAnimation::getName() const{
	return _name;
}

const std::vector<TransformQ>& SkeltalAnimation::getFrameCache() const{
	return _frameCache;
}
