## Tema 3 PCOM - Web client. REST API Communication

    First of all, this project starts with the implementation from lab9. The
following files are taken from there: buffer.c, buffer.h, helper.cpp, helper.hpp,
requests.cpp, requests.hpp. I did some changes in the helper files and the
requests files. In the helper files, I added: `is_natural_number`, function that
checks if a given string is a valid natural number , `get_status_Code`,
function that returns the HTTP status code from the response string `get_cookie`,
returns the value of the cookie header from the response string. In the response
files, I added: `compute_delete_request`, function that computes and returns a
DELETE request string and `compute_put_request`, function that computes and
returns a PUT request, used only in the `update_movie` function.
    Furthermore, I used the `nlohmann` json library to parse the JSON response,
I included the `json.hpp` file and the `json_fwd.hpp` file in the project for
this particular reason. I chose to use this library first of all because it was
recommended in the homework description and second of all because it is simple
to use and has a lot of functions designed to help you parse the JSON, which
is a big part of the project functionalities.
    The project is a web client that communicates with a REST API. The
implementation is based on commands read from stdin, that are executed
by the client, every command is a request to the server. The commands are:

- `login_admin`
    This function is called to log in as an admin. The username and password
    are read from stdin. First of all, the function checks if the username and
    password don't contain spaces, because that's considered invalid. Another
    error that is checked is if the admin is already logged in. The json payload
    is created and the cookies are copied in order to be sent in the request.
    Eventually, the POST request is sent to the server by using the
    `compute_post_request` function. Then the response is parsed to extract
    the status code. Depending on the status code, the function will print the
    SUCCESS or ERROR messages. If the login was successful, the cookie is updated.
    Otherwise, the json response is parsed to extract the error message and print it.
    Finally, the connection is closed and the resources are freed.

- `add_user`
    This function adds a new user by sending a POST request. It reads the
    username and password from stdin, checks for spaces and verifies that the
    current user is an admin. The json payload is created and the cookies are
    copied in order to be sent in the request. Eventually, the POST request is
    sent to the server by using the `compute_post_request` function. Then
    the response is parsed to extract the status code. Depending on the status
    code, the function will print the SUCCESS or ERROR messages. If the add
    user request was successful, the cookie is updated. Otherwise, the json
    response is parsed to extract the error message and print it. Finally,
    the connection is closed and the resources are freed.

- `get_users`
    This function returns the list of users by sending a GET request. It
    checks if the current user is an admin, copies cookies and sends the
    request by using the `compute_get_request` function. Then the response
    is parsed to extract the status code. On success, it prints the list of
    users from the response and updates the cookie. If the request fails,
    it prints the error message. Finally, the connection is closed and the
    resources are freed.

- `delete_user`
    This function deletes a user by sending a DELETE request. It reads the
    username from stdin, checks for spaces and verifies admin status. It
    builds the request path, by concatenating the username to the given path,
    copies cookies and sends the request, by using the `compute_delete_request`
    function. The response is parsed for the status code. On success, the cookie
    is updated and the success message is printed. On error, the error message
    is printed. The connection and resources are freed at the end.

- `logout_admin`
    This function logs out the admin by sending a GET request. It checks if
    the admin is logged in, copies cookies and sends the request. On success,
    it resets the admin status, clears cookies and token  and prints a success
    message. On error, it prints the error message.

- `login`
    This function logs in a user by sending a POST request. It reads the admin
    username, username, and password from stdin, checks for spaces, creates a
    JSON payload, copies cookies and sends the request, by using the
    `compute_post_request` function. The response is parsed for the status
    code. Depending on the status code, the function will print the SUCCESS or
    ERROR messages. If the login request was successful, the cookie is updated.
    Otherwise, the json response is parsed to extract the error message and
    print it.

- `get_access`
    This function retrieves the JWT access token by sending a GET request. It
    copies the cookies, sends the request and parses the response for the status
    code. If the request is successful, it saves the token and prints a success
    message and updates the cookies.

- `get_movies`
    This function returns the list of movies by sending a GET request. It
    copies the cookies and sends the request, by using the `compute_get_request`
    function It checks if the user has access to the library, by checking if
    there is a valid JWT token.  On success, it prints the list of
    movies from the response and updates the cookie. If the request fails,
    it prints the error message.

- `get_movie`
    This function returns the details of a movie by sending a GET request.
    It reads the movie id from stdin, checks for spaces and if the id is a
    natural number, builds the request path, copies cookies and sends the
    request, by using the `compute_get_request` function It checks if the user
    has access to the library, by checking if there is a valid JWT token.
    On success, it prints the details of the movie from the response and
    updates the cookie. If the request fails, it prints the error message.

- `add_movie`
    This function adds a new movie by sending a POST request. It reads the
    title, year, description and rating from stdin, checks for spaces,
    creates a JSON payload, copies cookies and sends the request, by using the
    `compute_post_request` function It checks if the user has access to the
    library, by checking if there is a valid JWT token. On success, it updates
    the cookie and prints a success message.

- `update_movie`
    This function updates a movie by sending a PUT request. It reads the id,
    title, year, description and rating from stdin, checks for spaces and if
    the id is a natural number, creates a JSON payload, copies cookies and
    sends the request, by using the `compute_put_request` function It checks
    if the user has access to the library, by checking if there is a valid JWT
    token. On success, it updates the cookie and prints a success message.
    On error, it prints the error message. Finally, the connection is closed
    and the resources are freed.

- `delete_movie`
    This function deletes a movie by sending a DELETE request. It reads the id
    from stdin, checks for spaces and if the id is a natural number,
    builds the request path, copies cookies and sends the request, by using the
    `compute_delete_request` function It checks if the userhas access to the
    library, by checking if there is a valid JWT token. On success, it updates the
    cookie and prints a success message.

- `add_collection`
    This function adds a new collection by sending a POST request. It reads
    the title and number of movies from stdin, then reads each movie ids,
    creates a JSON payload, copies cookies and sends the request to create
    the collection. It checks if the user has access to the library, by
    checking if there is a valid JWT token. Depending on the status code,
    the function will add movies to the collection by using the
    `add_collection_helper` function, that sends a POST request for each
    movie id. If not all the movies were added, it is considerd that the
    collection is not successfully created. The function uses a counter to
    keep track of the number of movies added. If all the movies were added,
    the collection is considered successfully created.

- `get_collections`
    This function returns the list of collections by sending a GET request.
    It copies the cookies and sends the request, by using the `compute_get_request`
    function It checks if the user has access to the library, by checking if
    there is a valid JWT token. On success, it prints the list of
    collections from the response and updates the cookie. If the request fails,
    it prints the error message.

- `get_collection`
    This function returns the details of a collection by sending a GET request.
    reads the collection id from stdin, checks for spaces and if the id is a
    natural number, builds the request path, copies cookies and sends the
    request, by using the `compute_get_request` function It checks if the user
    has access to the library, by checking if there is a valid JWT token.
    On success, it prints the details of the collection from the response and
    updates the cookie. If the request fails, it prints the error message.

- `delete_collection`
    This function deletes a collection by sending a DELETE request. It reads the id
    from stdin, checks for spaces and if the id is a natural number,
    builds the request path, copies cookies and sends the request, by using the
    `compute_delete_request` function It checks if the user has access to the
    library, by checking if there is a valid JWT token. On success, it updates the
    cookie and prints a success message.

- `add_movie_to_collection`
    This function adds a movie to a collection by sending a POST request. It
    reads the collection id and movie id from stdin, checks for spaces and if
    the ids are natural numbers, creates a JSON payload, copies cookies and
    sends the request, by using the `compute_post_request` function. It checks
    if the user has access to the library, by checking if there is a valid JWT
    token. On success, it updates the cookie and prints a success message.

- `delete_movie_from_collection`
    This function deletes a movie from a collection by sending a DELETE
    request. It reads the collection id and movie id from stdin, checks for
    spaces and if the ids are natural numbers, constructs the request path,
    copies cookies, and sends the request, by using the `compute_delete_request`
    function. It checks if the user has access to the library, by checking if
    there is a valid JWT token. On success, it updates the cookie and prints a
    success message.

- `logout`
    This function logs out a user by sending a GET request. It copies cookies,
    sends the request and parses the response for the status code. On
    success, it clears the cookies and token and prints a success message.Finally,
    the connection is closed and the resources are freed.
