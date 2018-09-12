#include <Util/Input.h>
#include <algorithm>

KeyCodeVector Input::_pressKeys;
KeyCodeVector Input::_beforePressKeys;

void Input::updateKeyInput(bool pressed, WPARAM wParam){

	if(pressed){
		_pressKeys.push_back(static_cast<UCHAR>(wParam));
	}
	else{
		_pressKeys.erase(std::remove(_pressKeys.begin(), _pressKeys.end(), wParam), _pressKeys.end());
	}
}

void Input::updatePerFrame(){

	_beforePressKeys.clear();
	_beforePressKeys = KeyCodeVector(_pressKeys.size());

	std::copy(_pressKeys.begin(), _pressKeys.end(), _beforePressKeys.begin());
}

bool Input::getKey(const KeyCode code){
	return Input::containtsKey(_pressKeys, code);
}

bool Input::getKeyDown(const KeyCode code){
	return ( !Input::containtsKey(_beforePressKeys, code) && Input::containtsKey(_pressKeys, code) );
}

bool Input::getKeyUp(const KeyCode code){
	return ( Input::containtsKey(_beforePressKeys, code) && !Input::containtsKey(_pressKeys, code) );
}

bool Input::containtsKey(KeyCodeVector & vector, const KeyCode code){
	
	const UCHAR uCode = static_cast<UCHAR>( code );
	return !( vector.end() == std::find_if(vector.begin(), vector.end(), [uCode](UCHAR &n){ return n == uCode; }) );
}
