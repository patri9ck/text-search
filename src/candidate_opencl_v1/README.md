Kernal Code:
To check if a query is equal to a substring of the given text we
create a 2D grind of workers. On one axis we have the for e.g 10 strings that we are looking for (queries).
On the other axis we have the text positions, so if the text contains 1000 characters, we have 1000 positions minus the
length of the query.
As soon as one character doesn't fit we quit the loop.
