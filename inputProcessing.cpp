#include "inputProcessing.h"

// Wait for user input before continuing. Used to stop program from closing outside of a command line.
void			haltExecution()
{
  std::cout << "Press enter to continue...";
  std::cin.get();
}

// Ask the user a question that they can answer via command line
bool			askYesNoQuestion(std::string question)
{
  std::string	inputString;

  std::cout << question << " (y/n) [Default: yes]: ";
  std::getline(std::cin, inputString);
  if (inputString == "n" || inputString == "no")
	return false;
  return true;
}

// Output things on the command line. Using shouldOutput this can be easily controlled globally
void			debugOutput(int timeStamp, std::string message,
							bool shouldOutput, bool finishLastOutput,
							bool finishLine)
{
  if (shouldOutput)
  {
	if (finishLastOutput)
	  std::cout << "Done! (" << std::right << std::setw(3) << time(NULL) - timeStamp << " seconds) " << endl;
	if (message != "")
	{
	  std::cout << std::left << std::setw(50) << message;
	  if (finishLine)
		std::cout << std::endl;
	}
  }
}


// Promt the User via command line to input his hero levels and return them as a vector<int>
std::vector<int>				takeHerolevelInput()
{
  std::vector<std::string>		stringLevels;
  std::vector<int>				levels{};
  std::string					input;
  std::fstream					heroFile;
  heroFile.exceptions(std::fstream::failbit);

  if (askYesNoQuestion("Do you want to load hero levels from file?"))
  {
	try
	{
	  heroFile.open("heroLevels" + heroVersion, std::fstream::in);
	  heroFile >> input;
	  stringLevels = split(input, ",");
	  for (size_t i = 0; i < stringLevels.size(); i++)
		levels.push_back(std::stoi(stringLevels[i]));
	  heroFile.close();
	}
	catch (const exception & e)
	{
	  std::cout << "Could not find Hero File or Hero File is deprecated. ";
	  std::cout << "(Were there new Heroes added recently?)" << std::endl;
	  std::cout << "Make sure you input the hero Levels manually at least once.";
	  std::cout << std::endl;
	  throw runtime_error("Hero File not found");
	}
  }
  else
  {
	std::cout << "Enter the level of the hero, whose name is shown ";
	std::cout << "(Enter 0 if you don't own the Hero)" << std::endl;
	for (size_t i = 0; i < baseHeroes.size(); i++)
	{
	  std::cout << baseHeroes[i].name << ": ";
	  std::getline(std::cin, input);
	  levels.push_back(std::stoi(input));
	}

	// Write Hero Levels to file to use next time
	heroFile.open("heroLevels" + heroVersion, std::fstream::out);
	for (size_t i = 0; i < levels.size() - 1; i++)
	  heroFile << levels[i] << ',';
	heroFile << levels[levels.size() - 1];
	heroFile.close();
	std::cout << "Hero Levels have been saved in a file. ";
	std::cout << "Next time you use this program you can load them from file.";
	std::cout << std::endl;
  }
  return levels;
}

// Promt the user via command Line to input a monster lineup and return them as a vector of pointers to those monster
std::vector<Monster *>			takeLineupInput(std::string prompt)
{
  std::vector<Monster *>		lineup{};
  std::string					questString = "quest";
  std::string					questString2 = "q";
  std::string					input;

  std::cout << prompt << std::endl;
  std::cout << "Enter Monsters separated with commas or type";
  std::cout << " f.e.quest17 to get the lineup for quest 17." << std::endl;
  std::getline(std::cin, input);

  if (input.compare(0, questString.length(), questString) == 0)
  {
	int questNumber = stoi(input.substr(questString.length(), 2));
	lineup = makeMonstersFromStrings(quests[questNumber]);
  }
  else if (input.compare(0, questString2.length(), questString2) == 0)
  {
	int questNumber = stoi(input.substr(questString2.length(), 2));
	lineup = makeMonstersFromStrings(quests[questNumber]);
  }
  else
  {
	vector<string> stringLineup = split(input, ",");
	lineup = makeMonstersFromStrings(stringLineup);
  }
  return lineup;
}

// Parse string linup input into actual monsters if there are heroes in the input, a leveled hero is added to the database
std::vector<Monster *>			makeMonstersFromStrings(std::vector<string> stringLineup)
{
  std::vector<Monster *>		lineup{};
  std::pair<Monster, int>		heroData;

  for (size_t i = 0; i < stringLineup.size(); i++)
  {
	if (stringLineup[i].find(":") != stringLineup[i].npos)
	{
	  heroData = parseHeroString(stringLineup[i]);
	  addLeveledHero(heroData.first, heroData.second);
	}
	lineup.push_back(monsterMap.at(stringLineup[i]));
  }
  return lineup;
}

// Parse hero input from a string into its name and level
std::pair<Monster, int>			parseHeroString(std::string heroString)
{
  std::string					name = heroString.substr(0, heroString.find(':'));
  Monster						hero;

  for (size_t i = 0; i < baseHeroes.size(); i++)
	if (baseHeroes[i].name == name)
	  hero = baseHeroes[i];
  int level = std::stoi(heroString.substr(heroString.find(':') + 1));
  return std::pair<Monster, int>(hero, level);
}

// Splits strings into a vector of strings. No need to optimize, only used for input.
std::vector<string>				split(std::string s, std::string to_split)
{
  std::vector<string>			output;
  size_t						x = 0;
  size_t						limit = 0;

  while (limit != s.npos)
  {
	limit = s.find(to_split, x);
	output.push_back(s.substr(x, limit - x));
	x = limit + to_split.length();
  }
  return output;
}

// Check if a string is exclusively a number.
bool							is_Number(std::string str)
{
  return !str.empty() && str.find_first_not_of("-+0123456789") == std::string::npos;
}