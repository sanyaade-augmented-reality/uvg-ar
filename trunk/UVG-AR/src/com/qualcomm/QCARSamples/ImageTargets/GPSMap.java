package com.qualcomm.QCARSamples.ImageTargets;



import java.util.List;

import android.content.Intent;
import android.graphics.drawable.Drawable;
import android.location.Criteria;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Bundle;
import android.view.Menu;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;

import com.google.android.maps.GeoPoint;
import com.google.android.maps.MapActivity;
import com.google.android.maps.MapController;
import com.google.android.maps.MapView;
import com.google.android.maps.Overlay;
import com.google.android.maps.OverlayItem;

public class GPSMap extends MapActivity implements LocationListener, OnClickListener{
	
	private MapView mapView;
	private Button mStartButton;
	
	@Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.gps_screen);
        
        // Getting reference to MapView
        mapView = (MapView) findViewById(R.id.map_view);
        
        // Setting Zoom Controls on MapView
        mapView.setBuiltInZoomControls(true);
        
        
        
        // Getting LocationManager object from System Service LOCATION_SERVICE
        LocationManager locationManager = (LocationManager) getSystemService(LOCATION_SERVICE);
        
        // Creating a criteria object to retrieve provider
        Criteria criteria = new Criteria();
        
        // Getting the name of the best provider
        String provider = locationManager.getBestProvider(criteria, true);
        
        // Getting Current Location
        Location location = locationManager.getLastKnownLocation(provider);
        
        if(location!=null){
        	onLocationChanged(location);
        }
        
        locationManager.requestLocationUpdates(provider, 20000, 0, this);
        
        mStartButton = (Button) findViewById(R.id.button_realidadaumentada);
        mStartButton.setOnClickListener(this);
        
    }
	
	private void startARActivity()
    {
        Intent i = new Intent(this, ImageTargets.class);
        startActivity(i);
    }


    public void onClick(View v)
    {
        switch (v.getId())
        {
        case R.id.button_realidadaumentada:
            startARActivity();
            break;
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.gps_screen, menu);
        return true;
    }

	@Override
	protected boolean isRouteDisplayed() {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public void onLocationChanged(Location location) {
		TextView tvLocation = (TextView) findViewById(R.id.tv_location);
		
		// Getting latitude
		double latitude = location.getLatitude();
		
		// Getting longitude
		double longitude = location.getLongitude();
		
		// Setting latitude and longitude in the TextView tv_location
		tvLocation.setText("Latitude:" +  latitude  + ", Longitude:"+ longitude );
		
		// Creating an instance of GeoPoint corresponding to latitude and longitude
		GeoPoint point = new GeoPoint((int)(latitude * 1E6), (int)(longitude*1E6));
		
		// Getting MapController
		MapController mapController = mapView.getController();
		
		// Locating the Geographical point in the Map
		mapController.animateTo(point);
		
		// Applying a zoom
		mapController.setZoom(15);
		
		// Redraw the map
		mapView.invalidate();
		
		// Getting list of overlays available in the map
		List<Overlay> mapOverlays = mapView.getOverlays();
		
		// Creating a drawable object to represent the image of mark in the map
		Drawable drawable = this.getResources().getDrawable(R.drawable.cur_position);
		
		// Creating an instance of ItemizedOverlay to mark the current location in the map
		CurrentLocationOverlay currentLocationOverlay = new CurrentLocationOverlay(drawable);
		
		// Creating an item to represent a mark in the overlay
		OverlayItem currentLocation = new OverlayItem(point, "Current Location", "Latitude : " + latitude + ", Longitude:" + longitude);

		// Adding the mark to the overlay
		currentLocationOverlay.addOverlay(currentLocation);
		
		// Clear Existing overlays in the map
		mapOverlays.clear();
		
		// Adding new overlay to map overlay
		mapOverlays.add(currentLocationOverlay);		
		
	}

	@Override
	public void onProviderDisabled(String provider) {
		// TODO Auto-generated method stub		
	}

	@Override
	public void onProviderEnabled(String provider) {
		// TODO Auto-generated method stub		
	}

	@Override
	public void onStatusChanged(String provider, int status, Bundle extras) {
		// TODO Auto-generated method stub		
	}

}
