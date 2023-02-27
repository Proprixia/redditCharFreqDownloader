# Reddit Scraper in C

---

## Project Overview 
This program is a scraper which uses the Reddit API to download the top 100 posts from each Subreddit in a text file, alongside all of their associated comments.
The scraper was built as part of an inquiry into whether and why different online communities exhibit different patterns in letter frequency.
The scraper was written entirely in C.
The program is licensed under the [GNU Public License version 3](https://www.gnu.org/licenses/gpl-3.0.en.html), or a later version at your option.

## Sister Analyzer Project
This program's [sister project](https://github.com/Proprixia/redditCharFrequencyAnalyzer) is a program which analyzes the letter frequencies in each downloaded Subreddit file and outputs them to a CSV. This project can be found at https://github.com/Proprixia/redditCharFrequencyAnalyzer.

## Setup
To use the scraper, please go through the [OAUTH2 process](https://github.com/reddit-archive/reddit/wiki/OAuth2) to obtain your own refreshable with your own bot. Add the access_token output to a file with a path matching the "tokenFile" you designated in your configuration file. Add your "username", "password", and "useragent" of your bot to a separate file which you designated as "loginFile", with each key ("username", "password", "useragent") spelled exactly as written here.

To use the program, create and/or edit the "config.json" file in the execution directory. Set important configuration values in the file, including:
+ "outDirectory": The directory to which you wish for the scraper program to output the downloaded subreddit text.
+ "subredditsFile": The path of the text file containing a newline-delineated list of subreddits you wish to download.
+ "tokenFile": The path of a JSON file containing the downloaded token for the program.
+ "loginFile": The path of a JSON file containing the login information (username, password, and user-agent) for your bot.

In addition, configure the following options:
+ "maxPostsPerSubreddit": The maximum number of posts the program will read from an individual subreddit.
+ "maxCommentsPerPost": The maximum number of comments the program will download from an individual post (if zero, this is unlmited).

Finish creating the necessary files to run the program, as written in your config file.

Next, import and setup the required libraries. These are [json-c](https://github.com/json-c/json-c) and [libcurl](https://curl.se/libcurl). These libraries are crucial to use the program.
To compile the source code into binary form using GCC, run "gcc -Werror -Wall main.c -l json-c -l curl -o scraper". You may also use a compiler of your choice.  Once you have completed these steps, operating the scraper is as simple as executing the final binary and finding the resulting files in the user-designated out directory.  ## Contact If you have any questions, comments, or concerns, please contact the sole developer at [Proprixia](https://github.com/proprixia) on GitHub. 

## Future 
In the future, I may add an automatic setup script, better feedback, and error-resistance.  

## Project Technical Description (for nerds)
The program source code consists of the following files:
+ main.c: Consists of the main program functions, including parsing the subreddit file (*main()*), requesting each subreddit (*getSubreddit()*), and requesting each post (*getPost()*).
+ net.c: The networking backbone of the program, containing facilities to request pages, without OAuth (*baseRequest()*) and using an OAuth wrapper function (*httpRequest()*). This file also contains supporting functions including obtaining the necessary token (*getToken()*), and a CURLOPT_WRITEFUNCTION function (*downloadToMem()*).
+ parseComments.c: File which does the hard job of scrolling through and parsing pages of comments, a task difficult enough to warrant its own file. Contains functions to parse all of the comments of a post given its JSON file (*parseComments()*), as well as support functions to manage memory for the previous function, including one to generate a JSON pointer string from an integer array (*genPathStr()*) and one to determine the number of digits in an integer (*genNumDigits()*).
+ macros.c: Contains useful macros for the project, mostly related to memory management and structures. *OUTPUTPATHARRAY* is used for debugging, printing the contents of a path array memory struct. *CREATEMEMSTRUCT*, *MEMSTRUCTCAT*, *CATSTRTOMEMORYSTRUCT*, and *FREEMEMSTRUCT* all manipulate memory structs in fairly self-explanatory ways. MEMSTRUCTCAT concatenates two existing memory structures, while CATSTRTOMEMSTRUCT concatenates a string to an existing memory structure. *CREATEMEMSTRUCT* creates a memory struct given a name and a type argument, while *FREEMEMSTRUCT* frees all memory held by a single memory struct. *GETINTARRAYIDX* returns the contents of the memory struct int array at a given index. Lastly, *COPYTOCOMMENTSTEXT* abstracts many of these features, directly appending a hard coded memory structure to another. This last macro is used exclusively in parseComments.c.
+ README.md: Contains useful information about the program. (You are here!)

In addition, the following JSON files contain useful information:
+ config.json: Contains file locations and behavior descriptions that the user can configure to edit the program's functionality. Explained in full in the [Setup](#setup) portion of this file.
+ loginFile: User-designated JSON containing "username", "password", and "useragent" key-value pairs which are necessary for access to the Reddit API. Both "username" and "password" are determined by the Reddit app you created yourself on your user account, set up in the [Reddit apps preferences page](https://old.reddit.com/prefs/apps/). Meanwhile, "useragent" should be formatted as "platform:publicAppName:versionString (by u/username)".
+ tokenFile: User-designated JSON containing all of the outputs of the OAuth setup for a refreshable token, as described in the Reddit API documentation. This includes the reusable "refresh_token", the once-daily refreshed "access_token", the "request_time" value which is used locally for determining if a new token is needed, and the static "token_type", "scope", and "expires_in" key-value pairs.

Lastly, the program relies on the following libraries:
+ [libcurl](https://curl.se/libcurl): Used to download files from the internet
+ [json-c](https://github.com/json-c/json-c): Used to parse local and remote JSON files.
+ C standard library: Uses, of course, many of the C standard library utilties for strings and memory, as well as occasional use for mathematical functions.