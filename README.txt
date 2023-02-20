This is a scraper that uses the Reddit API to download the top 100 posts from each Subreddit in a text file, "subreddits", alongside all of the comments.
The scraper was built as part of an inquiry into whether and why different online communities exhibit different patterns in letter frequency.
This program's sister project is a program which analyzes the letter frequencies in each downloaded Subreddit file and outputs them to a CSV. This project can be found at https://github.com/Proprixia/redditCharFrequencyAnalyzer
The program is licensed under the GNU Public License version 3, or a later version at your option.

To use the scraper, please go through the OAUTH2 process to obtain your own refreshable with your own bot. Add the access_token output to a file in the execution directory titled "token.json". Add your "username", "password", and "user-agent" of your bot to a separate file, "login.json".
Next, create a "/Out" directory to store the files you download from Reddit. This, too, should be in the execution directory. Create a "subreddits" file (case-sensitive) and list the name of each subreddit you wish to download on its own line.
To compile the source code into binary form using GCC, execute "gcc -Werror -Wall main.c -l json-c -l curl -o scraper".
Once you have completed these steps, operating is as simple as executing the final binary.
If you have any questions, comments, or concerns, please contact the developer at Proprixia on GitHub.

In the future, I may add an automatic setup script, built-in configuration options, and better feedback and error-resistance.
