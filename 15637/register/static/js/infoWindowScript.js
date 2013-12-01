function getInfoWindowHTML(item, statusQuery) {
	var id = item.pk;
	var fields = item.fields;
	var status = statusQuery.status;
	var start = new Date(Date.parse(fields.start_time));
	var end = new Date(Date.parse(fields.end_time));
	var isCreator = statusQuery.is_creator;
	var numParticipants = statusQuery.checkin_number
			+ statusQuery.joined_number + 1;

	var html = "<div class=\"MarkerPopUp\">";
	html += "<strong>Title: </strong>" + fields.name + "<br />";
	html += "<strong>Participants: </strong><span id=\"id_part\">" + numParticipants + "</span><br />";
	html += "<strong>Detail: </strong>" + fields.desc.substring(0, 50);
	if (fields.desc.length > 50) {
		html += "...";
	}
	html += "<br />";
	html += "<strong>Duration: </strong>";
	html += start.toLocaleDateString() + " " + start.toLocaleTimeString();
	html += " - ";
	html += end.toLocaleDateString() + " " + end.toLocaleTimeString();
	html += "<br />";
	html += "<strong>Location: </strong>" + fields.location + "<br />";
	html += "<div style=\"text-align:center; margin-top:20px;\">";

	html += "<a href=\"../../view/"
			+ id
			+ "\"><button class=\"btn btn-primary btn-large\" style=\"margin-right: 20px;\">View</button></a>";
	if (isCreator == true) {
		html += "<a href=\"../../edit/" + id
				+ "\"><button class=\"btn btn-primary btn-large\">Edit</button></a>";
	} else if (status == 1) {
		html += "<button id=\"joinBtn\" class=\"btn btn-primary btn-large\" onclick=\"changeStatus("
				+ id + ", 0)\">Join now</button>";
	} else if (status == 0) {
		html += "<button id=\"checkinBtn\" class=\"btn btn-primary btn-large\" onclick=\"changeStatus("
				+ id + ", 2)\">Check-in</button>";
	}
	html += "</div></div>";
	return html;
}

function changeStatus(id, type) {
	$.getJSON("../../status/change/" + id + "?status=" + type, function(resp) {
		if (resp.success == true) {
			if (type == 0) {
				$("#joinBtn").replaceWith(
						"<button id=\"checkinBtn\" class=\"btn btn-primary btn-large\" onclick=\"changeStatus("
								+ id + ", 2)\">Check-in</button>");
				$("#id_part").html(parseInt($("#id_part").html()) + 1);
			} else if (type == 2) {
				$("#checkinBtn").replaceWith("");
			}
		} else {
			alert("Failed: " + resp.error);
		}
	});
}