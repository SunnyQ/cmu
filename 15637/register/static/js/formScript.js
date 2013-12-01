function pwdResetFormInitialize() {
	$('#pwdreset-form').validate(
			{
				rules : {
					email : {
						required : true,
						email : true,
						maxlength : 75
					}
				},
				highlight : function(element) {
					$(element).closest('.control-group').removeClass('success')
							.addClass('error');
				},

				success : function(element) {
					element.text('OK!').addClass('valid').closest(
							'.control-group').removeClass('error').addClass(
							'success');
				}
			});

}

function registerFormInitialize() {
	$('#register-form').validate(
			{
				rules : {
					username : {
						minlength : 2,
						maxlength : 30,
						required : true
					},
					email : {
						required : true,
						email : true,
						maxlength : 75
					},
					password1 : {
						minlength : 8,
						required : true
					},
					password2 : {
						required : true,
						equalTo : "#id_password1"
					}
				},

				highlight : function(element) {
					$(element).closest('.control-group').removeClass('success')
							.addClass('error');
				},

				success : function(element) {
					element.text('OK!').addClass('valid').closest(
							'.control-group').removeClass('error').addClass(
							'success');
				}
			});
}

function pwdChangeFormInitialize() {
	$("#pwdchange-form").validate(
			{
				rules : {
					old_password : {
						minlength : 8,
						required : true
					},
					new_password1 : {
						minlength : 8,
						required : true
					},
					new_password2 : {
						required : true,
						equalTo : "#id_new_password1"
					},
				},

				highlight : function(element) {
					$(element).closest('.control-group').removeClass('success')
							.addClass('error');
				},

				success : function(element) {
					element.text('OK!').addClass('valid').closest(
							'.control-group').removeClass('error').addClass(
							'success');
				}
			});
}

function eventFormInitialize() {
	$('#startdatetimepicker').datetimepicker({
		format : 'yyyy-MM-dd hh:mm:ss',
		language : 'en-US',
		startDate : new Date(),
	});
	$('#enddatetimepicker').datetimepicker({
		format : 'yyyy-MM-dd hh:mm:ss',
		language : 'en-US',
		startDate : new Date(),
	});
	$('#event-form').validate(
			{
				rules : {
					name : {
						maxlength : 30,
						required : true
					},
					desc : {
						maxlength : 255,
						required : true
					},
					start_time : {
						required : true
					},
					end_time : {
						required : true
					},
					location : {
						required : true
					},
				},

				highlight : function(element) {
					$(element).closest('.control-group').removeClass('success')
							.addClass('error');
				},

				success : function(element) {
					element.text('OK!').addClass('valid').closest(
							'.control-group').removeClass('error').addClass(
							'success');
				}
			});
}

function validateEventForm() {
	var currentdate = new Date();
	var start = Date.parse($("#id_start_time").val());
	var end = Date.parse($("#id_end_time").val());

	var geocoder = new google.maps.Geocoder();
	geocoder
			.geocode(
					{
						'address' : $("#id_location").val()
					},
					function(results, status) {
						if (status == google.maps.GeocoderStatus.OK) {
							$("#id_lat")
									.val(results[0].geometry.location.lat());
							$("#id_lon")
									.val(results[0].geometry.location.lng());
							if (start <= currentdate) {
								alert("Start date must be greater than today");
								return false;
							} 
							
							if (end <= start) {
								alert("End date must be greater than start date");
								return false;
							}
							
							$("#event-form").unbind("submit");
							$("#event-form").submit();
						} else {
							alert("The location cannot be recognized!");
							return false;
						}
					});
	return false;
}


function editEventFormInitialize() {
	$('#startdatetimepicker').datetimepicker({
		format : 'yyyy-MM-dd hh:mm:ss',
		language : 'en-US',
		startDate : new Date(),
	});
	$('#enddatetimepicker').datetimepicker({
		format : 'yyyy-MM-dd hh:mm:ss',
		language : 'en-US',
		startDate : new Date(),
	});
	$('#edit-event-form').validate(
			{
				rules : {
					name : {
						maxlength : 30,
						required : true
					},
					desc : {
						maxlength : 255,
						required : true
					},
					start_time : {
						required : true
					},
					end_time : {
						required : true
					},
				},

				highlight : function(element) {
					$(element).closest('.control-group').removeClass('success')
							.addClass('error');
				},

				success : function(element) {
					element.text('OK!').addClass('valid').closest(
							'.control-group').removeClass('error').addClass(
							'success');
				}
			});
}


function appendEventBtn(eid) {
	$.getJSON("../../status/query/" + eid, function(resp) {
		if (resp.success) {
			var html = "<div class=\"row\" style=\"margin-top: 35px;\">";
			html += "<div class=\"span2 offset4\" style=\"text-align: right;\">";
			if (resp.status == 1) {
				html += "<button id=\"joinBtn\" class=\"btn btn-primary btn-large\" onclick=\"changeStatus(" + eid + ", 0)\">Join</button>";
			} else if (resp.status == 0) {
				html += "<button id=\"checkinBtn\" class=\"btn btn-primary btn-large\" onclick=\"changeStatus(" + eid + ", 2)\">Check-in</button>";
			}
			html += "</div></div>";
			$("#event-view").append(html);
		}
	});
}