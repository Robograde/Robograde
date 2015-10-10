Platforms
---------
* [Gitlab](https://gitlab.devtoi.eu)
* [Jenkins](http://hamnen06.comproj.bth.se:8090)
* [Slack](https://largegamebth.slack.com)

Getting started on Windows
--------------------------
1. Download [git bash](http://git-scm.com/downloads)
2. Start git bash.
3. If you don't have an ssh key you need to generate one with:
	1. `ssh-keygen -t rsa -C "someone@example.com"` (replace with your email).
	2. Put it in standard location.
	3. Give it a secure password.
	3. Copy all of the contents in the `id_rsa.pub` file located in `/c/Users/yourname/.ssh`.
	4. Go to the [ssh keys tab](https://gitlab.devtoi.eu/profile/keys) in your profile settings.
	5. Create a new key with a good name and paste the contents from the `id_rsa.pub` file into the key textbox.
4. Run `git clone ssh://gitlab@gitlab.devtoi.eu:19283/largegame/large-game.git`
5. `cd large-game` (the folder that you put the repository in)
6. `git fetch` fetches all branches.
7. `git checkout dev` This checks out the main developer branch, just stick with master if you want a runnable version of the game.
8. `cd proj` This is the main project folder.
9. `generateSolution.bat` This will generate a visual studio 2013 solution using cmake.
10. `explorer .` This will open the current folder in windows explorer.
11. Open _Robograde.sln_.
12. You should now be able to compile the projects.
13. In order to run the main executable you must set Robograde as the startup project by right clicking on it and selecting _Set as startup project_
14. Read the [wiki pages](https://gitlab.devtoi.eu/largegame/large-game/wikis/pages).

Getting start on Linux
----------------------
Run the windows steps 3, 4, 5, 6, 7 and 8. (In a terminal)
1. `cmake .` This generates a make file for the project.
2. `make` This compiles the project.
3. Run the executable with `../bin/linux/Robograde`
4. Read the [wiki pages](https://gitlab.devtoi.eu/largegame/large-game/wikis/pages).

Team members
------------
|	Name				|	Nickname	|	TODO acronym	|
| --------------------- | ------------- | ----------------- |
| David Pejtersen		|				| TODODP			|
| Johan Melin			| toi			| TODOJM			|
| Daniel Bengtsson		| Xar/MonzÜn	| TODODB			|
| Viktor Kelkkanen		| kelkka		| TODOVK			|
| Isak Almgren			| DrinQ			| TODOIA			|
| Henrik Johansson		| Exnotime		| TODOHJ			|
| Ola Enberg			| Deranes		| TODOOE			|
| Mattias Willemsen		|				| TODOMW			|
| Jens Stjernkvist		| 				| TODOJS			|
| Mostafa Hassan		| Musse			| TODOMH			|
| Richard Pettersson	| 				| TODORP			|
