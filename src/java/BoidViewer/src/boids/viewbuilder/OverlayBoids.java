package boids.viewbuilder;

import java.util.ArrayList;

public class OverlayBoids 
{
	ArrayList<Boid> _boids;

	public OverlayBoids(ArrayList<Boid> boids) 
	{
		_boids = boids;
	}

	public String exportToGnuplot() 
	{
		StringBuilder sb = new StringBuilder();
		
		for(Boid b : _boids)
		{
			sb.append(b.exportToGnuplot());
		}
		
		return sb.toString();
	}

}
