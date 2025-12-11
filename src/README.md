# Rolling Hash Approach

1. **Define Prime Number**  
   In this implementation, we use a fixed prime number: `131`.

2. **Calculate Query Hash**  
   To compute the `query_hash`, each character of the query is multiplied by powers of the prime number:
```
    query_hash = query[0]*prime^(m-1) + query[1]*prime^(m-2) + ... + query[m-1]
```
This represents the query as a number in base `prime`.

3. **Compute Power for Rolling Hash**  
   Calculate `power = prime^(m-1)` based on the length of the query.  
   This is necessary to correctly remove the leftmost character when rolling the hash over the text.

4. **Compute Hash for the First Window of the Text**  
   Calculate the hash of the first substring of the text with the same length as the query.

5. **Rolling Hash Across the Rest of the Text**  
   For each subsequent position:
- Remove the leftmost character:
  ```
  window_hash -= text[i-1] * power
  ```
- Shift the hash:
  ```
  window_hash *= prime
  ```
- Add the new rightmost character:
  ```
  window_hash += text[i + m - 1]
  ```

6. **Check for Hash Matches**  
   If the rolling hash matches `query_hash`, optionally perform a character-by-character comparison to confirm the match.
