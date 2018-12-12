// JavaScript Document
var Try = {
    these: function() {
        var returnValue;

        for (var i = 0; i < arguments.length; i++) {
            var lambda = arguments[i];
            try {
                returnValue = lambda();
                break;
            } catch(e) {}
        }

        return returnValue;
    }
}

function RequestURL(url) {
    var xmlhttp = Try.these(
    function() {
        return new ActiveXObject('Msxml2.XMLHTTP')
    },
    function() {
        return new ActiveXObject('Microsoft.XMLHTTP')
    },
    function() {
        return new XMLHttpRequest()
    }) || false;

    if (!xmlhttp)
        return "BE";

    xmlhttp.open("GET", url, false);
    xmlhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
    xmlhttp.setRequestHeader("Cache-Control", "no-cache");
    xmlhttp.send("");

    if (xmlhttp.readyState == 4) 
		if (xmlhttp.status == 200) {
        	return xmlhttp.responseText;
		} else {
        	return "SE";
		}

    return "CE";
}
