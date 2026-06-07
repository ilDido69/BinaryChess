#pragma once

#include "utils.h"

#include <iostream>
#include <ctime>
#include <map>
#include <string>
#include <array>
#include <cstdint>

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>

/*
Class for the gui
Future improvement:
- Game initialization with fen
- Menu with:
	bot on-off
	play button
- Timer
- Past moves
*/

class Game
{
private:
	sf::VideoMode videoMode;
	sf::RenderWindow* window;
	sf::Vector2u windowSize;

	bool endGame;

	float tileSize;
	sf::Vector2f boardStartPos;

	//set and clear tile selected
	void setSelection(int index);
	void clearSelection();

	//init functions
	void initVariables();
	void initWindow();
	void initTexturesAndSprites();

	void setText(Colors color);

	std::optional<sf::Font> font;
	std::optional<sf::Text> text;
	sf::SoundBuffer* soundBuffer;
	sf::Sound* sound;

	std::map<std::string, sf::Texture> textures;
	std::map<std::string, sf::Sprite*> sprites;
	std::map < Piece, std::string> pieceToKey;

	std::array<bool, 64> tilesToColor;
	int lastTileSelected;
	bool seePromotion;
	Move promotionMove;
	std::vector<Move> possibleMoveForPiece;
	int kingOnCheck;

	BoardState boardState;

	
public:
	//Constructors and Destructors
	Game();
	~Game();

	//Accessors

	//Modifires

	//functions
	const bool running() const;

	//update
	void poolEvents();
	void updateChecks(BoardState& boardState);
	void update();

	//render
	void renderTable();
	void renderPieces();
	void renderPromotion(int square);
	void render();
};

