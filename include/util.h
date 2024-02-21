#pragma once

/**
 * Return true if the string is valid for it to be a title name, false if not.
 * A string is valid if it contains at least an alphanumeric character
 */
int str_isValid(char* str);

/**
 * Return the source string stripped of any character that isn't alphanumeric
 * Each of those character is replaced with '_'
 */
char* str_strip(char* src);

/**
 * Return the extension of the file whose path is given
 * Return is of the form ".extension"
 * If there's no extension, return an empty string
 */
char* extension(char* path);

/**
 * Return 1 if the directory at path given exists
 * Return 0 if not
 */
int dirExist(char* path);