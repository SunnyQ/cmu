-- Q1-1
DROP TABLE IF EXISTS characters;
CREATE TABLE characters
(
char_id int primary key,
name char(25) not null
);


DROP TABLE IF EXISTS comics;
CREATE TABLE comics
(
comic_id int primary key,
name char(25) not null
);


DROP TABLE IF EXISTS appearances;
CREATE TABLE appearances
(
char_id int not null,
comic_id int not null,
primary key(char_id, comic_id)
);

.separator ' '
.import marvel_characters.txt characters
.import marvel_comic_books.txt comics
.import marvel.txt appearances

-- Q1-8
DROP INDEX IF EXISTS a_char;
CREATE INDEX a_char ON appearances(char_id);
DROP INDEX IF EXISTS a_comic;
CREATE INDEX a_comic ON appearances(comic_id);
DROP INDEX IF EXISTS char_index;
CREATE INDEX char_index ON characters(char_id);

SELECT name, count(*)
FROM co_actors
LEFT JOIN characters
ON id1 = char_id
GROUP BY id1
ORDER BY count(*) DESC
LIMIT 3;


