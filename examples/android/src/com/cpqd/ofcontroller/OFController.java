package com.cpqd.ofcontroller;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

import android.os.Bundle;
import android.os.Handler;
import android.app.Activity;
import android.text.method.ScrollingMovementMethod;
import android.view.ViewGroup.LayoutParams;
import android.widget.ScrollView;
import android.widget.TextView;

class Controller implements Runnable {
	static {
		System.loadLibrary("gnustl_shared");
		System.loadLibrary("ofcontroller");
	}

	public void run() {
		startController(6653, 4);
	}

	native void startController(int port, int nthreads);
}

public class OFController extends Activity {
	TextView tv;
	private int logUpdaterInterval = 1000;
	private Handler logUpdaterHandler;
	Runnable logUpdater = new Runnable() {
		@Override
		public void run() {
			updateLog();
			logUpdaterHandler.postDelayed(logUpdater, logUpdaterInterval);
		}
	};
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		Thread t = new Thread(new Controller());
		t.start();

		tv = new TextView(this);
		ScrollView sv = new ScrollView(this);
		sv.addView(tv);
		sv.setLayoutParams(new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT));
		tv.setMovementMethod(new ScrollingMovementMethod());
		setContentView(sv);

		logUpdaterHandler = new Handler();
		logUpdater.run();
	}

	void updateLog() {
		String pid = android.os.Process.myPid() + "";
		try {
			Process process = Runtime.getRuntime().exec(
					"logcat -d OFCONTROLLER:V *:S");
			BufferedReader bufferedReader = new BufferedReader(
					new InputStreamReader(process.getInputStream()));

			StringBuilder log = new StringBuilder();
			String line;
			while ((line = bufferedReader.readLine()) != null) {
				if (line.contains(pid)) {
					log.append(line.split(": ")[1] + "\n");
				}
			}
			tv.setText(log.toString());
		} catch (IOException e) {
		}
	}
}
