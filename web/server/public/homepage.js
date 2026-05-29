document.addEventListener("DOMContentLoaded", () => {

    //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
    // Define the 'request' function to handle interactions with the server
    function server_request(url, data={}, verb, callback) {
        return fetch(url, {
        credentials: 'same-origin',
        method: verb,
        body: JSON.stringify(data),
        headers: {'Content-Type': 'application/json'}
        })
        .then(response => response.json())
        .then(response => {
        if(callback)
            callback(response);
        })
        .catch(error => console.error('Error:', error));
    }

    let add_form = document.querySelector('form[name=add_user]');

    //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
    // Handle POST Requests
    add_form.addEventListener('submit', (event) => {
        // Stop the default form behavior
        event.preventDefault();

        //form data Validation
        let fname = document.getElementById("first_name").value;
        let lname = document.getElementById("last_name").value;
        let username = document.getElementById("username").value;
        let password = document.getElementById("password").value;
        if (fname.trim() == "") {
            alert("Please Enter a Valid First Name!!");
            return;
        }
        if (lname.trim() == "") {
            alert("Please Enter a Valid Last Name!!");
            return;
        }
        if (username.trim() == "") {
            alert("Please Enter a Valid Username!!");
            return;
        }
        if (password.trim() == "") {
            alert("Please Enter a Valid Password!!");
            return;
        }

        // Grab the needed form fields
        const action = add_form.getAttribute('action');
        const method = add_form.getAttribute('method');
        const data = Object.fromEntries(new FormData(add_form).entries());

        // Submit the POST request
        server_request(action, data, method, (response) => {

            // Clear the input form and bring focus to the first field again
            let inputs = add_form.querySelectorAll('input');
            for (let i = 0; i < inputs.length-1; i++) {
                inputs[i].value = '';
            }
            inputs[0].focus();
        });
    });



});

