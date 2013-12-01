$(document).ready(function() {
	if (document.getElementById("map-canvas")) {
		mapInitialize();
	}

	if (document.getElementById("register-form")) {
		registerFormInitialize();
	}

	if (document.getElementById("event-form")) {
		eventFormInitialize();
		$("#event-form").submit(validateEventForm);
		setAutoComplete(document.getElementById("id_location"));
	}

	if (document.getElementById("edit-event-form")) {
		editEventFormInitialize();
	}

	if (document.getElementById("pwdreset-form")) {
		pwdResetFormInitialize();
	}

	if (document.getElementById("pwdchange-form")) {
		pwdChangeFormInitialize();
	}

	if (document.getElementById("profile-form")) {
		setAutoComplete(document.getElementById("id_home_addr"));
	}

	if (document.getElementById("id_my_event")) {
		$('table tr').click(function() {
			window.location = $(this).attr('href');
			return false;
		});
	}
	
	if (document.getElementById("event-view")) {
		if (!document.getElementById("editBtn")) {
			appendEventBtn($("#id_eid").val());
		}
	}

});
