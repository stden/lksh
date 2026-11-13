// statements_generate
// written by Oleg [burunduk3] Davydov 39-232 (2009-08-21) at 11:44 UTC

#include <cassert>
#include <cstdio>
#include <cstring>

#include <string>
#include <utility>
#include <vector>

using std::pair;
using std::string;
using std::vector;

int usage()
{
  puts("statemets_generate — utility for creating contest statements");
  puts("usage: ./statemets_generate <input-file> <template-file>");
  puts("  usually input file is a file with contest information (see generic.src for details)");
  puts("  and output file is a TeX source for contest problems");
  return 1;
}

enum State
{
  STATE_INVALID,
  STATE_CONTEST,
  STATE_PROBLEMS
};

bool isAlpha( char ch )
{
  return ('a' <= ch && ch <= 'z') ||
         ('A' <= ch && ch <= 'Z') ||
         ('0' <= ch && ch <= '9') ||
          ch == '_' || ch == '.' || ch == '-';
}

State parseState( string s )
{
  // state: "^\[([a-z]*)\] *$"
  if (s[0] != '[')
    return STATE_INVALID;
  int i = 1;
  string state = "";
  for (; i < (int)s.length(); i++)
    if (isAlpha(s[i]))
      state += s[i];
    else
      break;
  if (s[i++] != ']')
    return STATE_INVALID;
  while (i < (int)s.length() && s[i] == ' ')
    i++;
  if (s[i++] != '\n')
    return STATE_INVALID;
  if (i < (int)s.length())
    return STATE_INVALID;
  if (state == "contest")
    return STATE_CONTEST;
  else if (state == "problems")
    return STATE_PROBLEMS;
  else
    return STATE_INVALID;
}

pair <string, string> parseAssignment( string s )
{
  string name = "", value = "";
  int i = 0;
  enum
  {
    MODE_NORMAL,
    MODE_SPACE,
    MODE_QUOTES
  } mode = MODE_NORMAL;
  for (; i < (int)s.length(); i++)
    if (isAlpha(s[i]))
      name += s[i];
    else
      break;
//  fprintf(stderr, "[debug] name = “%s”\n", name.c_str());
  if (name == "")
    return pair <string, string> ("", "");
  while (i < (int)s.length() && s[i] == ' ')
    i++;
  if (i >= (int)s.length() || s[i] != '=')
    return pair <string, string> ("", "");
  i++;
  while (i < (int)s.length() && s[i] == ' ')
    i++;
//  fprintf(stderr, "[debug] 1\n");
  for (; i < (int)s.length(); i++)
    if (mode == MODE_NORMAL)
    {
      if (isAlpha(s[i]))
        value += s[i];
      else if (s[i] == ' ')
        value += ' ', mode = MODE_SPACE;
      else if (s[i] == '\"')
        mode = MODE_QUOTES;
      else if (s[i] == '\n')
        break;
      else
      {
//        fprintf(stderr, "[debug] s[i=%d] = ‘%c’\n", i, s[i]);
        return pair <string, string> ("", "");
      }
    }
    else if (mode == MODE_SPACE)
    {
      if (s[i] != ' ')
        mode = MODE_NORMAL, i--;
    }
    else if (mode == MODE_QUOTES)
    {
      if (s[i] == '\"')
        mode = MODE_NORMAL;
      else if (s[i] == '\\')
      {
        i++;
        if (s[i] == '\\')
          value += '\\';
        else if (s[i] == '\"')
          value += '\"';
        else if (s[i] == 'n')
          value += '\n';
        else if (s[i] == '0')
          value += '\0';
        else if (s[i] == 't')
          value += '\t';
        else if (s[i] == 'r')
          value += '\r';
        else
          return pair <string, string> ("", "");
      }
      else
        value += s[i];
    }
  return make_pair(name, value);
}

char buffer[2048];

int main( int argc, char *argv[] )
{
  if (argc != 3)
    return usage();
  FILE *inputSrc = fopen(argv[1], "rt");
  assert(inputSrc != NULL);
  FILE *templateTeX = fopen(argv[2], "rt");
  assert(templateTeX != NULL);
  // 1. parce input file
  string contestName = "==contest-name==", contestLocation = "==contest-location==", contestDate = "==contest-date==";
  string output = "default-output";
  vector <string> problems, problemsID;
  State state = STATE_INVALID;
  while (fgets(buffer, 2047, inputSrc) == buffer)
  {
    State newState = parseState(buffer);
    if (newState != STATE_INVALID)
    {
      state = newState;
      continue;
    }
    pair <string, string> assignment = parseAssignment(buffer);
    if (assignment != pair <string, string> ("", ""))
    {
      if (state == STATE_CONTEST)
      {
        if (assignment.first == "contestName")
          contestName = assignment.second;
        else if (assignment.first == "contestLocation")
          contestLocation = assignment.second;
        else if (assignment.first == "contestDate")
          contestDate = assignment.second;
        else if (assignment.first == "output")
          output = assignment.second;
        else
          fprintf(stderr, "Warning: unknown field “%s”\n", assignment.first.c_str());
      }
      else if (state == STATE_PROBLEMS)
        problems.push_back(assignment.first), problemsID.push_back(assignment.second);
      continue;
    }
    if (buffer[0] != '\n')
    {
      buffer[strlen(buffer) - 1] = '\0';
      assert(fprintf(stderr, "Warning: line “%s” ommited\n", buffer) >= 0);
    }
  }
  FILE *outputTeX = fopen((output + ".tex").c_str(), "wt");
  assert(outputTeX != NULL);
  // 2. copy template.tex into output
  size_t length;
  while ((length = fread(buffer, 1, 2048, templateTeX)) != 0)
    assert(fwrite(buffer, 1, length, outputTeX) == length);
  // 3. output result
  fprintf(outputTeX, "\\contest{%%\n");
  fprintf(outputTeX, "%s%%\n", contestName.c_str());
  fprintf(outputTeX, "}{%%\n");
  fprintf(outputTeX, "%s%%\n", contestLocation.c_str());
  fprintf(outputTeX, "}{%%\n");
  fprintf(outputTeX, "%s%%\n", contestDate.c_str());
  fprintf(outputTeX, "}%%\n");
  fprintf(outputTeX, "\n");
  fprintf(outputTeX, "\\begin{document}\n");
  //%\raggedbottom
  for (int i = 0; i < (int)problems.size(); i++)
    assert(fprintf(outputTeX, "\\addProblem{%s}{%s}\n", problemsID[i].c_str(), problems[i].c_str()) >= 0);
  //%\pagebreak
  assert(fprintf(outputTeX, "\\end{document}\n") >= 0);
  assert(printf("%s", output.c_str()) >= 0);
  return 0;
}

