package boids.viewbuilder;

public class Boid 
{
	int _id;
	double _radius;
	Vector _pos;
	Vector _vel;
	
	public Boid(int id, double xpos, double ypos,
			double xvel, double yvel, double radius) 
	{
		_id = id;
		_radius = radius;
		_pos = new Vector(xpos, ypos);
		_vel = new Vector(xvel, yvel);
	}

	public Object exportToGnuplot() 
	{
		String me = this.arrow() + this.ball();
		return me;
	}

	private String ball() 
	{
		return "set object "+_id+" circle at "+_pos._x+","+_pos._y+
				" size scr 0.005 fc rgb \"navy\"\n";
	}

	private String arrow() 
	{
		return "set arrow "+_id+" from "+_pos._x+","+_pos._y+" to "+
				+(_pos._x+_vel._x)+","+(_pos._y+_vel._y)+"\n";
	}

}
