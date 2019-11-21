public class Hello {
	static{
		System.loadLibrary("hello");
	}
	public static native void hello();
	
    public static void main(String[] args) {
        hello();
    }
}