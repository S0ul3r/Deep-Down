#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <chrono>
#include <cmath>
#include <iostream>
#include <random>
#include <unordered_map>

using namespace std::chrono_literals;

// Random number generator
namespace
{
std::random_device randomSeed;	// Getting seed from random number engine
std::mt19937 gen(randomSeed()); // Mersenne Twister pseudo-random generator of 32-bit numbers seeded with randomSeed()
float random_float(float lower, float higher)
{
	std::uniform_real_distribution<float> dist(lower, higher);
	return dist(gen);
}
}

// Setting locals
static const int screenWidth = 1280;
static const int screenHeight = 720;
#define mapWidth 24
#define mapHeight 24
static const std::size_t texturesWidth = 256;
static const std::size_t texturesHeight = 256;
static const float visibility = 8.0f;
static const float fovAngle = 103;
static const float fov = fovAngle / 100.0f;
static const float sensitivity = 0.00125;
static const float minimapZoom = 0.5f;
static const float movementSpeed = 3.5f;
static const float brightness = 120.0f; //TO DO changing brightness
static const sf::Vector2i wallTextureIndexes[] = {
	sf::Vector2i { 0, 0 }, // 0
	sf::Vector2i { 2, 0 }, // 1
	sf::Vector2i { 3, 2 }, // 2
	sf::Vector2i { 0, 2 }, // 3
	sf::Vector2i { 1, 0 }, // 4
	sf::Vector2i { 1, 1 }, // 5
};

// Level locals
struct level
{
	sf::Vector2f spawnDirection;
	sf::Vector2f spawnPosition;
	sf::Vector2f teleportPosition;
	const int map[mapWidth][mapHeight];
};

// Setting spawn direction, player and teleport posision, and making map (TO DO maybe some kind of for loop with rand to automise preparing maps)
static const int levelStages = 3;
static const level levels[levelStages] = {
	{ sf::Vector2f(-1.f, 0.f),
		sf::Vector2f(22.f, 12.f),
		sf::Vector2f { 12.5f, 12.5f },
		{ { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
			{ 1, 0, 0, 0, 2, 0, 0, 0, 0, 0, 4, 0, 0, 3, 0, 0, 0, 5, 0, 0, 0, 1, 1, 1 },
			{ 1, 0, 0, 0, 2, 0, 0, 0, 0, 0, 4, 0, 0, 3, 0, 0, 0, 5, 0, 0, 0, 1, 1, 1 },
			{ 1, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 5, 0, 0, 0, 0, 0, 1 },
			{ 1, 0, 0, 0, 2, 0, 2, 2, 2, 2, 2, 0, 0, 0, 0, 3, 0, 5, 0, 3, 0, 0, 0, 1 },
			{ 1, 0, 0, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 1 },
			{ 1, 0, 0, 5, 5, 5, 2, 0, 0, 0, 2, 5, 5, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 1 },
			{ 1, 0, 0, 0, 0, 0, 2, 0, 0, 0, 2, 0, 5, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 1 },
			{ 1, 0, 0, 0, 0, 0, 2, 2, 0, 2, 2, 0, 5, 0, 5, 3, 0, 3, 0, 3, 0, 0, 0, 1 },
			{ 1, 0, 0, 0, 3, 0, 0, 0, 0, 0, 3, 0, 0, 0, 5, 0, 0, 0, 0, 3, 0, 0, 0, 1 },
			{ 1, 0, 0, 0, 3, 0, 0, 2, 2, 2, 3, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
			{ 1, 0, 0, 0, 3, 0, 0, 0, 0, 0, 3, 0, 2, 0, 5, 4, 4, 4, 0, 0, 5, 0, 5, 1 },
			{ 1, 0, 0, 2, 2, 2, 2, 0, 0, 0, 3, 0, 0, 0, 5, 0, 0, 0, 0, 0, 5, 0, 0, 1 },
			{ 1, 0, 0, 0, 0, 0, 2, 0, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 5, 0, 0, 1 },
			{ 1, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 1 },
			{ 1, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 5, 5, 0, 5, 0, 0, 1 },
			{ 1, 4, 4, 4, 0, 4, 4, 0, 4, 0, 0, 0, 2, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 1 },
			{ 1, 4, 0, 4, 0, 0, 0, 0, 4, 0, 0, 0, 2, 5, 5, 0, 4, 0, 3, 0, 0, 0, 0, 1 },
			{ 1, 4, 0, 0, 0, 0, 5, 0, 4, 2, 2, 2, 2, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 1 },
			{ 1, 4, 0, 4, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 3, 0, 1, 1 },
			{ 1, 4, 0, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 1, 1 },
			{ 1, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 3, 0, 1, 1 },
			{ 1, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
			{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 } } },
	{ sf::Vector2f(-1.f, 0.f),
		sf::Vector2f(22.f, 12.f),
		sf::Vector2f { 5.5f, 5.5f },
		{ { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
			{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
			{ 1, 0, 0, 0, 3, 3, 3, 3, 3, 3, 0, 5, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
			{ 1, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 5, 5, 5, 5, 5, 1 },
			{ 1, 0, 0, 0, 3, 0, 5, 0, 0, 0, 0, 3, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
			{ 1, 0, 0, 0, 3, 0, 5, 5, 5, 5, 0, 3, 0, 3, 3, 3, 0, 2, 0, 0, 0, 3, 0, 1 },
			{ 1, 1, 1, 0, 3, 3, 5, 0, 0, 5, 0, 3, 0, 0, 0, 3, 0, 2, 2, 2, 2, 3, 0, 1 },
			{ 1, 0, 0, 0, 0, 0, 5, 0, 0, 5, 4, 4, 4, 0, 0, 3, 0, 0, 0, 0, 2, 0, 0, 1 },
			{ 1, 0, 0, 2, 2, 2, 5, 0, 0, 5, 0, 0, 4, 4, 0, 3, 2, 2, 2, 2, 2, 0, 0, 1 },
			{ 1, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 3, 0, 0, 0, 0, 0, 0, 0, 1 },
			{ 1, 0, 0, 0, 0, 0, 0, 0, 4, 4, 0, 0, 0, 4, 0, 3, 0, 0, 4, 4, 4, 4, 4, 1 },
			{ 1, 1, 1, 1, 0, 1, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 3, 0, 0, 1 },
			{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, 2, 2, 2, 2, 0, 2, 0, 0, 0, 3, 0, 0, 1 },
			{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 2, 4, 4, 4, 5, 5, 0, 1 },
			{ 1, 0, 2, 2, 2, 1, 3, 3, 0, 0, 2, 0, 0, 0, 4, 0, 2, 0, 0, 0, 0, 0, 0, 1 },
			{ 1, 0, 2, 0, 0, 3, 3, 0, 0, 0, 2, 0, 3, 0, 4, 0, 0, 0, 0, 4, 4, 4, 4, 1 },
			{ 1, 0, 2, 2, 0, 3, 0, 0, 4, 0, 2, 0, 0, 0, 4, 0, 3, 0, 0, 4, 0, 0, 0, 1 },
			{ 1, 0, 0, 2, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 4, 0, 3, 0, 0, 0, 0, 5, 0, 1 },
			{ 1, 0, 0, 2, 0, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 0, 3, 0, 0, 4, 0, 0, 0, 1 },
			{ 1, 0, 0, 2, 0, 4, 0, 4, 4, 0, 0, 0, 0, 0, 0, 0, 3, 2, 2, 2, 2, 0, 0, 1 },
			{ 1, 0, 0, 2, 0, 4, 4, 4, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 1 },
			{ 1, 0, 0, 0, 0, 0, 0, 4, 0, 0, 5, 0, 0, 0, 0, 0, 3, 0, 0, 0, 2, 0, 0, 1 },
			{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 1 },
			{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 } } },
	{ sf::Vector2f(-1.f, 0.f),
		sf::Vector2f(22.f, 12.f),
		sf::Vector2f { 1.5f, 16.5f },
		{ { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
			{ 1, 0, 3, 3, 0, 0, 0, 0, 2, 0, 0, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 0, 0, 1 },
			{ 1, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 4, 3, 3, 3, 3, 3, 2, 0, 1 },
			{ 1, 3, 0, 3, 2, 2, 0, 0, 2, 0, 5, 5, 0, 1, 0, 4, 0, 0, 0, 0, 0, 2, 0, 1 },
			{ 1, 0, 0, 3, 0, 2, 0, 0, 2, 0, 0, 5, 0, 1, 0, 4, 0, 4, 4, 4, 4, 2, 0, 1 },
			{ 1, 0, 3, 3, 0, 2, 2, 2, 2, 2, 0, 5, 0, 1, 0, 0, 0, 0, 0, 0, 0, 2, 0, 1 },
			{ 1, 0, 0, 3, 0, 0, 0, 0, 0, 2, 0, 5, 5, 5, 5, 5, 5, 2, 2, 2, 0, 0, 0, 1 },
			{ 1, 0, 0, 3, 3, 3, 3, 3, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 4, 4, 1 },
			{ 1, 0, 0, 0, 0, 0, 0, 3, 0, 2, 4, 4, 4, 4, 4, 4, 3, 3, 0, 2, 0, 4, 0, 1 },
			{ 1, 3, 0, 5, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 4, 0, 0, 3, 0, 2, 0, 4, 0, 1 },
			{ 1, 0, 0, 5, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 4, 0, 1 },
			{ 1, 0, 0, 0, 0, 0, 5, 5, 5, 3, 3, 3, 3, 0, 4, 5, 0, 1, 1, 1, 1, 1, 0, 1 },
			{ 1, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1 },
			{ 1, 3, 3, 3, 0, 5, 5, 5, 5, 5, 5, 0, 2, 0, 4, 0, 0, 1, 0, 0, 0, 1, 0, 1 },
			{ 1, 0, 0, 0, 0, 5, 0, 0, 0, 0, 5, 0, 0, 3, 3, 3, 3, 3, 0, 4, 0, 1, 0, 1 },
			{ 1, 0, 0, 0, 0, 5, 0, 0, 0, 0, 5, 0, 0, 4, 0, 0, 0, 0, 0, 4, 0, 0, 0, 1 },
			{ 1, 0, 0, 2, 2, 5, 0, 5, 5, 5, 5, 0, 0, 4, 0, 2, 0, 1, 0, 4, 0, 1, 0, 1 },
			{ 1, 0, 0, 2, 0, 5, 0, 5, 0, 4, 0, 0, 4, 4, 0, 2, 0, 1, 0, 4, 0, 1, 0, 1 },
			{ 1, 0, 0, 2, 0, 5, 0, 5, 0, 4, 4, 4, 4, 4, 0, 2, 0, 1, 0, 4, 3, 3, 3, 1 },
			{ 1, 0, 2, 2, 0, 3, 0, 5, 0, 0, 0, 0, 4, 0, 0, 2, 0, 1, 1, 1, 0, 0, 0, 1 },
			{ 1, 0, 2, 2, 0, 3, 0, 5, 5, 5, 0, 0, 4, 4, 4, 4, 0, 0, 0, 3, 0, 3, 3, 1 },
			{ 1, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 3, 0, 0, 0, 1 },
			{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 5, 0, 0, 0, 0, 0, 4, 0, 4, 0, 0, 0, 0, 0, 1 },
			{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 } } },
};

namespace
{
// Mathhhhh translating angles to vectors stackoverflow help
static const sf::Vector2f TOP(0, -1);
static const sf::Vector2f TOP_RIGHT(1, -1);
static const sf::Vector2f TOP_LEFT(-1, -1);
static const sf::Vector2f BOTTOM(0, 1);
static const sf::Vector2f BOTTOM_RIGHT(1, 1);
static const sf::Vector2f BOTTOM_LEFT(-1, 1);
static const sf::Vector2f RIGHT(1, 0);
static const sf::Vector2f LEFT(-1, 0);
// For converting radians to degrees
constexpr float radiansToDegreesConst = 57.295779513082320876798154814105f;
//Needed for setting up Field of View
static sf::Vector2f angleToVector(const float& degree)
{
	if (degree == 0 || degree == 360)
		return TOP;
	if (degree == 180)
		return BOTTOM;
	if (degree == 90)
		return RIGHT;
	if (degree == 270)
		return LEFT;
	if (degree == 45)
		return TOP_RIGHT;
	if (degree == 135)
		return BOTTOM_RIGHT;
	if (degree == 225)
		return BOTTOM_LEFT;
	if (degree == 315)
		return TOP_LEFT;

	const float rad = (-degree + 90.0f) / radiansToDegreesConst;
	return sf::Vector2f(cosf(rad), -sinf(rad));
}
// Easier math for setting up teleport and making hitbox for it
static float magnitude(const sf::Vector2f& a)
{
	return sqrt(a.x * a.x + a.y * a.y);
}
// More vector math, constants from trygonometry atan2
static float vectorToAngle(const sf::Vector2f& vec)
{
	if (vec.x == 0.0f && vec.y == 0.0f)
		return 0.0f;
	/** http://math.stackexchange.com/questions/1098487/atan2-faster-approximation
	* atan2(y,x)
	* a := min (|x|, |y|) / max (|x|, |y|)
	* s := a * a
	* r := ((-0.0464964749 * s + 0.15931422) * s - 0.327622764) * s * a + a
	* if |y| > |x| then r := 1.57079637 - r
	* if x < 0 then r := 3.14159274 - r
	* if y < 0 then r := -r
	**/
	const float absx = std::abs(vec.x), absy = std::abs(vec.y);
	const float a = absx > absy ? absy / absx : absx / absy;
	const float s = a * a;
	float r = ((-0.0464964749 * s + 0.15931422) * s - 0.327622764) * s * a + a;

	if (absy > absx)
		r = 1.57079637 - r;
	if (vec.x < 0)
		r = 3.14159274 - r;
	if (vec.y < 0)
		r = -r;

	float ang = r * radiansToDegreesConst + 90.0f;
	if (ang < 0.0f)
		ang += 360.0f;
	else if (ang > 360.0f)
		ang -= 360.0f;
	return ang + 90;
}
// Easier color settin'
static void setColor(sf::Vertex& v, unsigned char value)
{
	v.color.r = v.color.g = v.color.b = value;
}

static void setColor(sf::Vertex& v, unsigned char value, unsigned char alpha)
{
	setColor(v, value);
	v.color.a = alpha;
}
}
// Make the game darker
static void setBrightness(sf::Vertex& v, const float distance, const float maxDist, const float myBrightness = brightness)
{
	const float dark = std::max(std::min(myBrightness * distance / maxDist, myBrightness), 0.0f);
	const float brightness = myBrightness - dark;
	setColor(v, brightness);
}

static sf::Vector2f getTexturePlacement(const sf::Vector2i& texindexX)
{
	return sf::Vector2f(texindexX.x * (texturesWidth + 2) + 1, texindexX.y * (texturesHeight + 2) + 1);
}

static sf::Vector2f getTexturePlacement(const int type)
{
	return getTexturePlacement(wallTextureIndexes[type]);
}

int main()
{
	// Sound in background
	sf::Music backgroundSound;
	backgroundSound.openFromFile("src/Media/backgroundSound.ogg");
	backgroundSound.setLoop(true);
	backgroundSound.play();

	// Player breathing
	sf::Music breathing;
	breathing.openFromFile("src/Media/breathing.ogg");
	breathing.setLoop(true);
	breathing.play();
	breathing.setPitch(1.5f);

	// Player creepy background
	sf::Music creepyBack;
	creepyBack.openFromFile("src/Media/laughSingingScream.ogg");
	creepyBack.setLoop(true);
	creepyBack.play();

	// Steps sounds
	std::unordered_map<std::string, sf::SoundBuffer> soundBuffers;
	soundBuffers["step_1"].loadFromFile("src/Media/step_1.wav");
	soundBuffers["step_2"].loadFromFile("src/Media/step_2.wav");
	soundBuffers["step_3"].loadFromFile("src/Media/step_3.wav");
	std::vector<sf::Sound> playingSounds;

	auto playSound = [&playingSounds, &soundBuffers](const std::string& name, float pitch = 1.0f, float volume = 1.0f) {
		playingSounds.emplace_back(soundBuffers[name]);
		sf::Sound& snd = playingSounds.back();
		snd.setPitch(pitch);
		snd.setVolume(volume * 50.0f);
		snd.play();
	};

	auto soundCleanup = [&playingSounds]() {
		for (auto iter = playingSounds.begin(); iter != playingSounds.end();)
		{
			if (iter->getStatus() == sf::Sound::Stopped)
				iter = playingSounds.erase(iter);
			else
				++iter;
		}
	};

	auto sound_destroy = [&backgroundSound, &creepyBack, &playingSounds, &soundCleanup]() {
		backgroundSound.stop();
		creepyBack.stop();

		for (auto& snd : playingSounds)
			snd.stop();

		soundCleanup();
	};

	// Make main window
	sf::RenderWindow window(sf::VideoMode(screenWidth, screenHeight), "Deep Down");
	window.setVerticalSyncEnabled(false);
	window.setMouseCursorVisible(false);
	window.setMouseCursorGrabbed(true);
	sf::Vector2i windowCenter { screenWidth / 2, screenHeight / 2 };

	// Prepare render texture
	sf::RenderTexture renderTexture;
	renderTexture.create(screenWidth, screenHeight);
	sf::Sprite renderTextureSprite(renderTexture.getTexture());

	// Teleport
	sf::Texture teleportTexture;
	teleportTexture.setSmooth(true);
	teleportTexture.loadFromFile("src/Media/teleport.png");
	sf::Sprite teleportSprite(teleportTexture);
	teleportSprite.setOrigin(teleportTexture.getSize().x * 0.5f, teleportTexture.getSize().y * 0.5f);
	teleportSprite.setPosition(teleportSprite.getOrigin());
	sf::RenderTexture teleportRenderTexture;
	teleportRenderTexture.create(teleportTexture.getSize().x, teleportTexture.getSize().y);

	//Zaxis used to make "3D" from 2D
	float ZDistance[screenWidth];

	// Wall textures
	sf::Texture texture;
	texture.setSmooth(true);
	texture.loadFromFile("src/Media/walls.png");

	// Compass Textures
	sf::Texture compassTexture;
	compassTexture.setSmooth(true);
	compassTexture.loadFromFile("src/Media/compass.png");
	sf::Texture compassShadowTexture;
	compassShadowTexture.setSmooth(true);
	compassShadowTexture.loadFromFile("src/Media/compassShadow.png");
	sf::Texture compassCircuitTexture;
	compassCircuitTexture.setSmooth(true);
	compassCircuitTexture.loadFromFile("src/Media/compassCircuit.png");
	sf::Texture compassArrowTexture;
	compassArrowTexture.setSmooth(true);
	compassArrowTexture.loadFromFile("src/Media/compassArrow.png");

	// Minimap render texture
	const float minimapHeight = compassShadowTexture.getSize().y;
	sf::RenderTexture minimapRenderTexture;
	minimapRenderTexture.create(minimapHeight / minimapZoom, minimapHeight / minimapZoom);
	minimapRenderTexture.setRepeated(false);
	minimapRenderTexture.setSmooth(true);

	// Minimap circle
	sf::CircleShape minimapCircle(minimapHeight * 0.5);
	minimapCircle.setTexture(&minimapRenderTexture.getTexture());
	minimapCircle.setOrigin(minimapCircle.getRadius(), minimapCircle.getRadius());
	minimapCircle.setPosition({ 10 + minimapHeight * 0.5f, screenHeight - minimapHeight * 0.5f - 10 });
	minimapCircle.setFillColor(sf::Color(255, 255, 255, 200));

	// Minimap square size
	const float squareSize = static_cast<float>(minimapRenderTexture.getSize().y) / mapWidth;

	// compass
	sf::Sprite compass(compassTexture);
	compass.setOrigin(compassTexture.getSize().x / 2, compassTexture.getSize().y / 2);
	compass.setPosition(minimapCircle.getPosition());

	// Compass circle
	sf::Sprite compassCircle(compassCircuitTexture);
	compassCircle.setOrigin(compassCircuitTexture.getSize().x / 2, compassCircuitTexture.getSize().y / 2);
	compassCircle.setColor(sf::Color(70, 70, 70));
	compassCircle.setPosition(minimapCircle.getPosition());

	// Compass shadow
	sf::Sprite compassShadow(compassShadowTexture);
	compassShadow.setOrigin(compassShadowTexture.getSize().x / 2, compassShadowTexture.getSize().y / 2);
	compassShadow.setPosition(minimapCircle.getPosition());

	// Compass arrow
	sf::Sprite compassArrow(compassArrowTexture);
	compassArrow.setOrigin(compassArrowTexture.getSize().x / 2, compassArrowTexture.getSize().y / 2);
	compassArrow.setScale(1.0f, 1.5f);

	// Arrays
	sf::VertexArray ceiling(sf::Quads, 4);
	sf::VertexArray floor(sf::Quads, 4);
	sf::VertexArray minimapSquares { sf::Quads, mapWidth * mapHeight * 4 };
	sf::VertexArray minimapCone { sf::Triangles, 3 };
	sf::VertexArray wallStructures(sf::Lines, screenWidth * 2);
	sf::VertexArray teleportLines(sf::Lines);
	sf::Vertex teleportVertex;

	// Clock
	sf::Clock clock;

	// Variables
	int levelID = -1;
	float positionX, positionY;	  // X and Y start position
	float directionX, directionY; // Initial direction vector
	float aerialX, aerialY;		  // The 2d ray caster version of camera plane

	// Move camera
	auto moveCamera = [&directionX, &directionY, &aerialX, &aerialY](const float mouseMove, const float constant) {
		const float rotSpeed = mouseMove * constant; // Constant value is in radians/second

		// Both camera direction and camera plane must be rotated
		const float olddirectionX = directionX;
		directionX = directionX * cos(-rotSpeed) - directionY * sin(-rotSpeed);
		directionY = olddirectionX * sin(-rotSpeed) + directionY * cos(-rotSpeed);
		const float oldaerialX = aerialX;
		aerialX = aerialX * cos(-rotSpeed) - aerialY * sin(-rotSpeed);
		aerialY = oldaerialX * sin(-rotSpeed) + aerialY * cos(-rotSpeed);
	};

	// Human walk swing
	float walkSwing;
	float walkingTimer = 0.0f;
	sf::Vector2i previousMousePosition = sf::Mouse::getPosition(window);

	// Game loop, refreshing the game, added lag for safety avoiding sheneningans in timestep loop
	// https://en.sfml-dev.org/forums/index.php?topic=20405.0
	//delta for player speed, set breathing and walk swing.
	const float delta = 0.008f;
	std::chrono::nanoseconds timestep(8ms), lag(0ns);
	auto time_start = std::chrono::high_resolution_clock::now();
	float fullTimer = 0.0f;

	// Setting level, assign current, next level for teleport and debug ("H")
	auto currentLevel = [&levelID] { return levels[levelID]; };

	auto setLevel = [&levelID, &currentLevel, &positionX, &positionY, &directionX, &directionY, &aerialX, &aerialY](int id) {
		levelID = std::min(std::max(id, 0), levelStages);

		positionX = currentLevel().spawnPosition.x;
		positionY = currentLevel().spawnPosition.y;
		directionX = currentLevel().spawnDirection.x;
		directionY = currentLevel().spawnDirection.y;
		aerialX = 0;
		aerialY = fov;
	};

	auto nextLevel = [&levelID, &setLevel] {
		int next = levelID + 1;
		if (next >= levelStages)
			next = 0;
		setLevel(next);
	};

	// Load with first level
	setLevel(0);

	while (window.isOpen())
	{
		// Setup variables for the loop,
		// https://en.sfml-dev.org/forums/index.php?topic=20405.msg146788#msg146788
		auto now = std::chrono::high_resolution_clock::now();
		lag += now - time_start;
		time_start = now;

		// Unlimited update loop
		while (lag >= timestep)
		{
			lag -= timestep;
			fullTimer += delta;

			// SFML events
			{
				sf::Event event {};
				while (window.pollEvent(event))
				{
					// Close game on Esc
					if (event.type == sf::Event::Closed || (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape))
						window.close();
					// Evaluate mouse movement comparing last position with current posision of the coursor
					else if (event.type == sf::Event::MouseMoved)
					{
						sf::Vector2i now { event.mouseMove.x, event.mouseMove.y };
						moveCamera(sensitivity * (now.x - previousMousePosition.x), 1);
						previousMousePosition = now;
					}
				}

				// Center mouse
				sf::Mouse::setPosition(previousMousePosition = windowCenter, window);
			}

			{
				// Character movement
				{
					sf::Vector2f givenDirection((sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) - (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left)),
						(sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::W)) - (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::S)));

					bool isMoving = givenDirection.x != 0 || givenDirection.y != 0;

					// Walking sway for immersion xd
					{
						//Check for movement
						bool isMoving = givenDirection.x != 0 || givenDirection.y != 0;
						if (isMoving)
							walkingTimer += delta;
						float calculateSin = sin(15.0f * walkingTimer + 2.0f * fullTimer);
						walkSwing = screenHeight * (isMoving ? 0.016 : 0.008) * calculateSin;

						// Walking sound effect
						// When bobbing reaches to bottom, play walk sound once
						static bool walkSoundPlayed = false;
						if (calculateSin < -0.98f)
						{
							if (!walkSoundPlayed)
							{
								// get random step sound from loaded files
								walkSoundPlayed = true;
								if (isMoving)
									playSound("step_" + std::to_string(int(std::floor(random_float(1, 3 + 1)))), random_float(0.75f, 1.25f));
							}
						}
						else
							walkSoundPlayed = false;
					}

					// Breathing for standing still and movement
					{
						const float nextBreathe = isMoving ? 2.f : 1.f;
						float currentBreathe = breathing.getPitch();
						if (nextBreathe != currentBreathe)
						{
							float newBreathe = currentBreathe + (nextBreathe - currentBreathe) * delta;
							if (newBreathe != currentBreathe)
								breathing.setPitch(newBreathe);
						}
					}

					// Movement for forward or back and diagonal
					{
						// Set current movement speed, delta
						float moveSpeed = delta * movementSpeed;

						// If we are moving diagonally get correct movement speed (diagonal of the square)
						if (givenDirection.x != 0 && givenDirection.y != 0)
							moveSpeed /= sqrt(2);

						// Using movement speed we are setting up our posision
						auto move = [&positionX, &positionY, &currentLevel](const sf::Vector2f& vec) {
							static const float coveredDistance = 12.0f;
							sf::Vector2i nowPosistion(positionX, positionY);
							sf::Vector2i nextPosition(positionX + coveredDistance * vec.x, positionY + coveredDistance * vec.y);
							if (currentLevel().map[nextPosition.x][nowPosistion.y] == 0)
								positionX += vec.x;
							if (currentLevel().map[nowPosistion.x][nextPosition.y] == 0)
								positionY += vec.y;
						};

						// Foreward and backwards
						if (givenDirection.y != 0)
						{
							auto dir = sf::Vector2f(directionX, directionY) * givenDirection.y;
							move(dir * moveSpeed);
						}

						// Left and right movement
						if (givenDirection.x != 0)
						{
							auto dir = sf::Vector2f { directionY, -directionX } * givenDirection.x;
							move(dir * moveSpeed);
						}
					}

					// Teleport collision
					{
						auto inHitbox = currentLevel().teleportPosition - sf::Vector2f(positionX, positionY);
						if (magnitude(inHitbox) < 0.5f)
							nextLevel();
					}
				}

				// Update minimap rotation
				{
					minimapCircle.setRotation(vectorToAngle({ directionX, directionY }));
					compass.setRotation(minimapCircle.getRotation());
				}

				// Update teleport
				{
					teleportSprite.setRotation(teleportSprite.getRotation() + 120.0f * delta);
				}
			}

			// Debugging levels
			static bool previousKey = false;
			bool currentKey = sf::Keyboard::isKeyPressed(sf::Keyboard::H);
			if (currentKey && !previousKey)
				nextLevel();
			previousKey = currentKey;

			// Wipe sounds
			soundCleanup();
		}

		// Render preparation
		{
			// Creating walls using ray casting
			{
				// Raycasting algorithm, Loop every vertical line of the screen
				// Mostly translated from JS to C++, idea is used in DOOM games and thats what I used to reference and learning
				// from here:
				// https://github.com/ammonb/stereogram-raycaster
				// https://dev.opera.com/articles/3d-games-with-canvas-and-raycasting-part-1/
				// https://permadi.com/1996/05/ray-casting-tutorial-table-of-contents/
				// https://lodev.org/cgtutor/index.html

				/*
				for (var x = 0; x < SCREEN_WIDTH; x++) {
				camera_heading + FOV * x / SCREEN_WIDTH;
				var angle = angleForColumn(x);

				// calculate distance to wall from camera position at given angle
				var [color, distance] = castRay(angle);

				// draw wall slice
				var h = WALL_HEIGHT / distance;
				drawLine(x, SCREEN_HEIGHT/2 - h, x, SCREEN_HEIGHT/2 + h, color);
				}
				*/

				for (int x = 0, indexWallVectorX = 0; x < screenWidth; ++x, indexWallVectorX += 2) {

					// Calculate ray position and direction, translated from:
					// https://github.com/williamblair/williamblair.github.io/blob/376117c88ee7774b229ec33533f9510ee9878e31/templates/raycasting.html
					const float cameraCoorX = 2.0f * x / screenWidth - 1; // x coordinate in camera space

		// Using camera, direction vector and cmaera "plane" we get ray direction coords
		const float rayDirectionX = directionX + aerialX * cameraCoorX;
		const float rayDirectionY = directionY + aerialY * cameraCoorX;

		// caltulating distance Length of ray from one x or y-side to next x or y-side
		const float deltaDistanceX = sqrt(1 / (rayDirectionX * rayDirectionX));
		const float deltaDistanceY = sqrt(1 + (rayDirectionX * rayDirectionX) / (rayDirectionY * rayDirectionY));

		// length of ray from current position to next x or y side
		float sideDistanceX;
		float sideDistanceY;

		// left or right for x and y side
		int stepX;
		int stepY;

		// get our current position box
		int mapX = positionX;
		int mapY = positionY;

		// height is the original wall height divided by the distance to the wall (or length of the ray cast for each column).
		// calculate step and initial side dist
		// Rendering in horizontal direction
		if (rayDirectionX < 0)
		{
			stepX = -1;
			sideDistanceX = (positionX - mapX) * deltaDistanceX;
		}
		else
		{
			stepX = 1;
			sideDistanceX = (mapX + 1.0f - positionX) * deltaDistanceX;
		}

		// Rendering in vertical direction
		if (rayDirectionY < 0)
		{
			stepY = -1;
			sideDistanceY = (positionY - mapY) * deltaDistanceY;
		}
		else
		{
			stepY = 1;
			sideDistanceY = (mapY + 1.0f - positionY) * deltaDistanceY;
		}

		// perform DDA (digital differential analysis)
		int side;	 // North/South or East/West wall hit?
		int hit = 0; // true once we've hit a wall
		float perpWallDist;

		while (hit == 0)
		{
			// Jump to the next square
			if (sideDistanceX < sideDistanceY)
			{
				sideDistanceX += deltaDistanceX;
				mapX += stepX;
				side = 0;
			}
			else
			{
				sideDistanceY += deltaDistanceY;
				mapY += stepY;
				side = 1;
			}

			// check if we hit a wall
			if (currentLevel().map[mapX][mapY] > 0)
				hit = 1;
		}

		// calculate distance projected on camera direction
		if (side == 0)
		{
			perpWallDist = std::fabs((mapX - positionX + (1 - stepX) / 2) / rayDirectionX);
		}
		else
		{
			perpWallDist = std::fabs((mapY - positionY + (1 - stepY) / 2) / rayDirectionY);
		}

		// Calculate height of line to draw on screen
		const float lineHeight = std::abs(screenHeight / perpWallDist);

		// Calculate lowest and highest pixel to fill in current stripe added mine walkSwing (changes height)
		const float drawStart = walkSwing - lineHeight * 0.5f + screenHeight * 0.5f;
		const float drawEnd = walkSwing + lineHeight * 0.5f + screenHeight * 0.5f;

		// Calculate value of wallLocationX, we want to know from our position where did we hit the wall
		float wallLocationX;
		if (side == 0)
			wallLocationX = positionY + perpWallDist * rayDirectionY;
		else
			wallLocationX = positionX + perpWallDist * rayDirectionX;
		wallLocationX -= std::floor((wallLocationX));

		// X coordinate on the texture
		int textureLocationX = wallLocationX * texturesWidth;
		if (side == 0 && rayDirectionX > 0)
			textureLocationX = texturesWidth - textureLocationX - 1;
		if (side == 1 && rayDirectionY < 0)
			textureLocationX = texturesHeight - textureLocationX - 1;

		// Save distance to ZDistance
		ZDistance[x] = perpWallDist;

		// Prepare wall structures, bottom left and top right corners
		{
			auto base = getTexturePlacement(currentLevel().map[mapX][mapY]);
			wallStructures[indexWallVectorX + 0].texCoords = base + sf::Vector2f(textureLocationX, 0);
			wallStructures[indexWallVectorX + 1].texCoords = base + sf::Vector2f(textureLocationX, texturesHeight);
			wallStructures[indexWallVectorX + 0].position = sf::Vector2f(x, drawStart);
			wallStructures[indexWallVectorX + 1].position = sf::Vector2f(x, drawEnd);

			// Brightness depending how close/far are we from the wall
			const float distance = magnitude(sf::Vector2f(mapX - positionX + (side == 1 ? wallLocationX : 0), mapY - positionY + (side == 0 ? wallLocationX : 0)));
			setBrightness(wallStructures[indexWallVectorX + 0], distance, visibility);
			setBrightness(wallStructures[indexWallVectorX + 1], distance, visibility);
		}
	}
}

// Minimap
{
	// Tiles
	{
		for (int minimapX = 0, indexX = 0; minimapX < mapWidth; ++minimapX)
		{
			for (int minimapY = 0; minimapY < mapHeight; ++minimapY, indexX += 4)
			{
				const int type = currentLevel().map[minimapY][minimapX];

				// Position
				minimapSquares[indexX + 0].position = { minimapX * squareSize, minimapY * squareSize };
				minimapSquares[indexX + 1].position = { (minimapX + 1) * squareSize, minimapY * squareSize };
				minimapSquares[indexX + 2].position = { (minimapX + 1) * squareSize, (minimapY + 1) * squareSize };
				minimapSquares[indexX + 3].position = { minimapX * squareSize, (minimapY + 1) * squareSize };

				// Texture
				const auto base = getTexturePlacement(type);
				minimapSquares[indexX + 0].texCoords = { base.x, base.y };
				minimapSquares[indexX + 1].texCoords = { base.x + texturesWidth, base.y };
				minimapSquares[indexX + 2].texCoords = { base.x + texturesWidth, base.y + texturesHeight };
				minimapSquares[indexX + 3].texCoords = { base.x, base.y + texturesHeight };

				// Color
				const float dark = 100;
				unsigned char color = type == 0 ? dark : 255;
				for (int i = 0; i < 4; ++i)
					setColor(minimapSquares[indexX + i], color);
			}
		}
	}

	// Player Field of View
	const sf::Vector2f playerPosition { positionY * squareSize, positionX * squareSize };
	const float playerDirection = -vectorToAngle({ directionX, directionY });
	{
		// Map angles left and right side
		const float mapViewDistance = squareSize * visibility * (minimapHeight / minimapRenderTexture.getSize().x) / minimapZoom;
		const sf::Vector2f leftConeSide = angleToVector(playerDirection - fovAngle * 0.5f) * mapViewDistance;
		const sf::Vector2f rightConeSide = angleToVector(playerDirection + fovAngle * 0.5f) * mapViewDistance;

		// FOV triangle positions
		minimapCone[0].position = playerPosition;
		minimapCone[1].position = minimapCone[0].position + leftConeSide;
		minimapCone[2].position = minimapCone[0].position + rightConeSide;

		// Most visible on our arrow
		const unsigned char color = 200;
		setColor(minimapCone[0], color, 50);

		// GOes more transparent when we get further from arrow
		for (int i = 1; i <= 2; ++i)
			setColor(minimapCone[i], color, 0);
	}

	// Compass Arrow
	{
		compassArrow.setPosition(playerPosition);
		compassArrow.setRotation(playerDirection);
	}

	// Size of minimap based on rectabgle and setting it's position
	{
		const sf::Vector2f minimapRenderTextureSize(minimapRenderTexture.getSize().x, minimapRenderTexture.getSize().y);
		const sf::Vector2f minimapBaseCoords = squareSize * -(0.5f * sf::Vector2f { mapWidth, mapHeight } + sf::Vector2f { -positionY, -positionX });
		const sf::Vector2i minimapSize(minimapZoom * minimapRenderTextureSize.x, minimapZoom * minimapRenderTextureSize.y);

		minimapCircle.setTextureRect(sf::IntRect(
			std::min(std::max(minimapBaseCoords.x + (1 - minimapZoom) * 0.5f * minimapRenderTextureSize.x, 0.0f), minimapRenderTextureSize.x - minimapSize.x),
			std::min(std::max(minimapBaseCoords.y + (1 - minimapZoom) * 0.5f * minimapRenderTextureSize.y, 0.0f), minimapRenderTextureSize.y - minimapSize.y),
			minimapSize.x,
			minimapSize.y));
	}
}

// Ceiling
{
	ceiling[0].position = { 0.0f, walkSwing };
	ceiling[1].position = { screenWidth, walkSwing };
	ceiling[2].position = { screenWidth, walkSwing + screenHeight * 0.51f };
	ceiling[3].position = { 0.0f, walkSwing + screenHeight * 0.51f };
	ceiling[2].color = ceiling[3].color = sf::Color { 20, 10, 7 };
	ceiling[0].color = ceiling[1].color = sf::Color { 3, 6, 15 };
}

// Floor
{
	floor[0].position = { 0.0f, walkSwing + screenHeight * 0.5f };
	floor[1].position = { screenWidth, walkSwing + screenHeight * 0.5f };
	floor[2].position = { screenWidth, walkSwing + screenHeight * 1.1f };
	floor[3].position = { 0.0f, walkSwing + screenHeight * 1.1f };
	sf::Color c { 25, 24, 23 };
	floor[2].color = floor[3].color = c;
	floor[0].color = floor[1].color = c;
}

// Teleport
// https://lodev.org/cgtutor/raycasting3.html#Scale
{
	// Cleanup
	teleportLines.clear();

	// Convert teleport position Translate sprite position to relative to camera
	float spriteX = currentLevel().teleportPosition.x - positionX;
	float spriteY = currentLevel().teleportPosition.y - positionY;

	// constant for correct matrix calculations
	float fixingVal = 1.0f / (aerialX * directionY - directionX * aerialY);

	float transformX = fixingVal * (directionY * spriteX - directionX * spriteY);
	float transformY = fixingVal * (-aerialY * spriteX + aerialX * spriteY);

	int spriteScreenX = int((screenWidth / 2) * (1 + transformX / transformY));

	// Get height of the teleport sprite on the screen
	int spriteHeight = std::abs(int(screenHeight / (transformY))); // Using "transformY" instead of the real distance prevents fish-eye
	// Calculate lowest and highest pixel to fill in current stripe
	int drawStartY = -spriteHeight / 2 + screenHeight / 2;
	if (drawStartY < 0)
		drawStartY = 0;
	int drawEndY = spriteHeight / 2 + screenHeight / 2;
	if (drawEndY >= screenHeight)
		drawEndY = screenHeight - 1;

	// Get width of the teleport sprite
	int spriteWidth = std::abs(int(screenHeight / (transformY)));
	int drawStartX = -spriteWidth / 2 + spriteScreenX;
	if (drawStartX < 0)
		drawStartX = 0;
	int drawEndX = spriteWidth / 2 + spriteScreenX;
	if (drawEndX >= screenWidth)
		drawEndX = screenWidth - 1;

	// Loop for scaling our sprite, so it changes when we move
	// https://lodev.org/cgtutor/raycasting3.html#Scale
	for (int stripe = drawStartX; stripe < drawEndX; stripe++)
	{
		int textureLocationX = int(256 * (stripe - (-spriteWidth / 2 + spriteScreenX)) * teleportRenderTexture.getSize().x / spriteWidth) / 256;
		// The conditions in the if are:
		// 1) It's in front of camera plane so you don't see things behind you
		// 2) It's on the screen (left)
		// 3) It's on the screen (right)
		// 4) ZDistance, with perpendicular distance
		if (transformY > 0 && stripe > 0 && stripe < screenWidth && transformY < ZDistance[stripe])
		{
			int topD = drawStartY * 256 - screenHeight * 128 + spriteHeight * 128;
			int topTextureY = ((topD * teleportRenderTexture.getSize().y) / spriteHeight) / 256;

			int bottomD = drawEndY * 256 - screenHeight * 128 + spriteHeight * 128;
			int bottomTextureY = ((bottomD * teleportRenderTexture.getSize().y) / spriteHeight) / 256;

			// Top
			teleportVertex.texCoords = sf::Vector2f(textureLocationX, topTextureY);
			teleportVertex.position = sf::Vector2f(stripe, walkSwing + drawStartY);
			teleportLines.append(teleportVertex);

			// Bottom
			teleportVertex.texCoords = sf::Vector2f(textureLocationX, bottomTextureY);
			teleportVertex.position = sf::Vector2f(stripe, walkSwing + drawEndY);
			teleportLines.append(teleportVertex);
		}
	}
}
}

// Renders
{
	// Cleanup
	{
		window.clear(sf::Color::Black);
		renderTexture.clear(sf::Color::Black);
	}

	// Draw everything
	{
		// Draw ceiling
		{
			renderTexture.draw(ceiling);
}

// Draw floor
{
	renderTexture.draw(floor);
}

// Draw walls
{
	renderTexture.draw(wallStructures, &texture);
}

// Teleport
{
	// Animating teleport rotation
	{
		teleportRenderTexture.clear(sf::Color::Transparent);
teleportRenderTexture.draw(teleportSprite);
teleportRenderTexture.display();
}

// Draw teleport from lines
{
	renderTexture.draw(teleportLines, &teleportRenderTexture.getTexture());
}
}

// Draw Minimap
{
	// Cleanup
	{
		minimapRenderTexture.clear(sf::Color::Black);
	}

	// Draw Squares
	{
		minimapRenderTexture.draw(minimapSquares, &texture);
	}

	// Draw FOV cone
	{
		minimapRenderTexture.draw(minimapCone);
	}

	// Draw compass arrow
	{
		minimapRenderTexture.draw(compassArrow);
	}

	// Draw minimap circle
	{
		// Finalize minimap render texture
		minimapRenderTexture.display();

		// Draw minimap circle
		renderTexture.draw(minimapCircle);
	}

	// Draw rest of compass
	{
		renderTexture.draw(compassShadow);
		renderTexture.draw(compass);
		renderTexture.draw(compassCircle);
	}
}
}

// Last render and window render
{
	renderTexture.display();
	window.draw(renderTextureSprite);
	window.display();
}
}
}

//cleanup sound
sound_destroy();

return EXIT_SUCCESS;
}
