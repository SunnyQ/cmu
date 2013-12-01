var map;
var lastOpenedWindow;
var eventMarkerArr = [];

function mapInitialize() {
	map = configMap();
	if (navigator.geolocation) {
		navigator.geolocation.getCurrentPosition(function(position) {
			initialLocation = new google.maps.LatLng(position.coords.latitude,
					position.coords.longitude);
			$("#id_addressbar").val(initialLocation.toString());
			setFocus(initialLocation, initialLocation.lat() + ", "
					+ initialLocation.lng());
			addEvents(initialLocation.lat(), initialLocation.lng());
		}, function() {
			alert("GeoLocation Service is failed...");
			setFocus(new google.maps.LatLng(-34.397, 150.644),
					"-34.397, 150.644");
		});
	} else {
		alert("GeoLocation Service is disabled...");
		setFocus(new google.maps.LatLng(-34.397, 150.644), "-34.397, 150.644");
	}
}

function addEvents(lat, lng) {
	$
			.getJSON(
					"?lat=" + lat + "&lon=" + lng + "&radius="
							+ $("#id_radius").val(),
					function(msg) {
						var events = $.parseJSON(msg.events);
						$
								.each(
										events,
										function(index, item) {
											$
													.getJSON(
															"../../status/query/"
																	+ item.pk,
															function(
																	statusQuery) {
																var marker = new google.maps.Marker(
																		{
																			map : map,
																			position : getMarkerPlace(item.fields.geo_location),
																			draggable : false
																		});

																eventMarkerArr
																		.push(marker);

																if (item.fields.status == 0) {
																	marker
																			.setIcon('http://maps.google.com/mapfiles/ms/icons/yellow-dot.png');
																} else if (item.fields.status == 1) {
																	marker
																			.setIcon('http://maps.google.com/mapfiles/ms/icons/blue-dot.png');
																}

																buildInfoWindow(
																		getInfoWindowHTML(
																				item,
																				statusQuery),
																		marker);
															});
										});
					});
}

function getMarkerPlace(point) {
	point = point.substring(point.indexOf("(") + 1, point.indexOf(")"));
	return new google.maps.LatLng(point.split(" ")[1], point.split(" ")[0]);
}

function setFocus(initialLocation, address) {
	map.setCenter(initialLocation);

	for ( var i = 0; i < eventMarkerArr.length; i++) {
		eventMarkerArr[i].setMap(null);
	}

	eventMarkerArr = []
	var marker = new google.maps.Marker({
		map : map,
		position : initialLocation,
		draggable : false
	});

	eventMarkerArr.push(marker);
	buildInfoWindow("Current location: " + address, marker);
}

function buildInfoWindow(info, marker) {
	var infowindow = new google.maps.InfoWindow({
		content : info,
		size : new google.maps.Size(25, 25)
	});

	google.maps.event.addListener(marker, 'click', function() {
		if (lastOpenedWindow != null)
			lastOpenedWindow.close();
		infowindow.open(map, marker);
		lastOpenedWindow = infowindow;
	});
}

function configMap() {
	var mapOptions = {
		zoom : 12,
		zoomControlOptions : {
			style : google.maps.ZoomControlStyle.SMALL,
			position : google.maps.ControlPosition.LEFT_CENTER
		},
		mapTypeId : google.maps.MapTypeId.ROADMAP,
		streetViewControl : false,
		panControl : false,
		mapTypeControl : false
	}
	var map = new google.maps.Map(document.getElementById("map-canvas"),
			mapOptions);

	setAutoComplete(document.getElementById("id_addressbar"));
	return map;
}

function setAutoComplete(addressBar) {
	var options = {
		types : [ 'geocode' ]
	};
	autocomplete = new google.maps.places.Autocomplete(addressBar, options);
}

function runScript(e) {
	if (e == null || e.keyCode == 13) {
		var address = $("#id_addressbar").val();
		var geocoder = new google.maps.Geocoder();
		geocoder.geocode({
			'address' : address
		}, function(results, status) {
			if (status == google.maps.GeocoderStatus.OK) {
				setFocus(results[0].geometry.location,
						results[0].formatted_address);
				addEvents(results[0].geometry.location.lat(), results[0].geometry.location.lng());
			} else {
				alert("Geocode was not successful for the following reason: "
						+ status);
			}
		});
	}
}