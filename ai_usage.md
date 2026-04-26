\# Prompt 1

Prompt : In C code, write a function : int parse\_condition(const char \*input, char \*field, char \*op, char \*value); which splits a field:operator:value string into its three parts. Supported fields: severity, category, inspector, timestamp. Supported operators: ==, !=, <, <=, >, >=.

Deepseek AI result : a correct parse\_condition function, it forgot to check if the value string is too large which is weird since it did this check for field and op.
What I changed : The values for the maximum length of strings were way too large ( 256 ) so I lowered them to 20, also added the max length check to value.

What I learned : That you can create this function without strtok, I excepted AI to use strtok since that function is good at breaking down string with separators.



\# Prompt 2

Prompt : Write a function : int match\_condition(report \*r, const char \*field, const char \*op, const char \*value); which returns 1 if the record satisfies the condition and 0 otherwise. Structure of report : typedef struct report{ int id; char inspector\[20]; float lat; float longit; char category\[20]; int severity; time\_t timestamp; char text\[50]; }report;. Supported fields: severity, category, inspector, timestamp. Supported operators: ==, !=, <, <=, >, >=.

Deepseek AI result : a correct match\_condition function, just a big block of ifs and else ifs, nothing more to say.

What I changed : Removed some unnecesarry comments.

What I learned : This function wouldn't have been hard to build, just time consuming and AI can save some a lot of time sometimes. 

Not even AI uses switch case and enum ( this is a joke ).

