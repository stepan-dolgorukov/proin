#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int amount_arguments,
         char* arguments[])
{
  if (amount_arguments < 2)
  {
    return 0;
  }

  for (int position = 1; position < amount_arguments; ++position)
  {
    int identifier = atoi(arguments[position]);
    char command[50] = {'\0'};

    sprintf(command, "insmod proin.ko identifier_process=%i", identifier);

    FILE* file_output = popen(command, "r");

    if (file_output == NULL)
    {
      continue;
    }

    int status = pclose(file_output);

    if (status != 0)
    {
      fprintf(stderr, "No information about the process %i\n", identifier);

      continue;
    }

    FILE* file_response = fopen("/proc/proin_response", "r");

    if (file_response == NULL)
    {
      printf("%s\n", "fopen");

      file_output = popen("rmmod proin", "r");
      status = pclose(file_output);

      if (status != 0)
      {
        fprintf(stderr, "Unsuccessfull module remove\n");

        return 1;
      }

      fprintf(stderr, "No information about the process %i\n", identifier);
      continue;
    }

    char response[300] = {'\0'};

    fread(response, 1, sizeof response, file_response);

    char* field = strtok(response, ";");
    int position_field = 0;

    while (field != NULL)
    {
      if (position_field == 0)
      {
        int identifier_response = atoi(field);

        if (identifier_response != identifier)
        {
          fprintf(stderr, "Mismatched response for process %i\n", identifier);

          file_output = popen("rmmod proin", "r");
          status = pclose(file_output);

          if (status != 0)
          {
            fprintf(stderr, "Unsuccessfull module remove\n");

            return 1;
          }

          return 1;
        }

        printf("identifier = %s\n", field);
      }

      else if (position_field == 1)
      {
        printf("user = %s\n", field);
      }

      else if (position_field == 2)
      {
        printf("executable = %s\n", field);
      }

      else
      {
        fprintf(stderr, "Undefined field\n");
      }

      field = strtok(NULL, ";");
      ++position_field;
    }

    fclose(file_response);
    file_output = popen("rmmod proin", "r");
    status = pclose(file_output);

    if (status != 0)
    {
      fprintf(stderr, "Unsuccessfull module remove\n");

      return 1;
    }
  }

  return 0;
}
