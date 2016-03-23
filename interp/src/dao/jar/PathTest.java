package dao.jar;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.fail;

import org.junit.Test;

public class PathTest {

	@Test
	public void testLengthMake() {
		FuncPath[] paths = new FuncPath[11];
		for (int i = 0; i < paths.length; i++)
			paths[i] = new FuncPath(i);
		for (int i = 0; i < FuncPath.CELLDEPTH; i++)
			assertEquals(1, paths[i].data.length);
		for (int i = 0; i + FuncPath.CELLDEPTH < paths.length; i++)
			assertEquals(1 << i, paths[FuncPath.CELLDEPTH + i].data.length);
	}
	
	private FuncPath funcPath;
	
	@Test
	public void testAllocMerge() {
		funcPath = new FuncPath(6);
		
		assertEquals(Math.max(1, 1 << 6 - FuncPath.CELLDEPTH), funcPath.data.length);
		
		assertEquals(1, funcPath.len);
		
		for (int i = 1; i <= 6; i++) {
			funcPath.DOALC();
			assertEquals(1 << i, funcPath.len);
			assertEquals(i, funcPath.lenDepth);
			assertEquals(1 << i, funcPath.alloc);
			assertEquals(i, funcPath.allocDepth);
		}
		
		try {
			funcPath.DOALC();
			fail("No exception.");
		} catch (OutOfMemoryError e) {
		} catch (Exception e) {
			fail("Wrong exception.");
		}
		
	}
	
	

}
