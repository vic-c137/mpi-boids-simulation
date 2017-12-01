package boids.viewbuilder;

import java.util.ArrayList;
import java.util.HashMap;

public class BoidModel 
{
	private HashMap<Integer,ArrayList<Boid>> _map = new HashMap<Integer,ArrayList<Boid>>();
	private int _iteration;
	private int _loops;


	public BoidModel(int loops) 
	{
		this._iteration = 0;
		this._loops = loops;

		
		for( int i = 0; i < _loops; i++ )
			_map.put(i, new ArrayList<Boid>());
	}

	public void addBoid(Boid newBoid, int p) 
	{
		ArrayList<Boid> list = _map.get(p);
		list.add(newBoid);
	}

	public OverlayBoids update() 
	{
		OverlayBoids _overlay = new OverlayBoids(_map.get(_iteration));
		if(_iteration==_loops)
			_iteration=0;
		else
			_iteration++;
		return _overlay;
	}

}
