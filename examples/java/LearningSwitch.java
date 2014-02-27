import java.math.BigInteger;
import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.Map;
import java.util.Arrays;

import fluid.base.*;
import fluid.msg.*;

public class LearningSwitch extends OFServer {
	static {
		System.loadLibrary("fluid_base");
		System.loadLibrary("fluid_msg_of10");
	}

	static short test(short bytes) {
		return bytes;
	}

	static Map<Integer, Map<Long, Integer>> table = new HashMap<Integer, Map<Long, Integer>>();

	@Override
	public void connection_callback(OFConnection conn,
			OFConnection.Event event_type) {
		System.out.println("Connection id=" + conn.get_id() + " event="
				+ event_type);
	}

	private Long get_long_mac(byte[] mac) {
		ByteBuffer br = ByteBuffer.allocate(8);
		br.put(mac);
		br.putShort((short) 0x0000);
		br.flip();
		return br.getLong();
	}

	private void flood(OFConnection conn, PacketIn pi) {
		OutputAction act = new OutputAction(0xfffb, 1024);
		PacketOut msg = new PacketOut(pi.xid(), pi.buffer_id(), pi.in_port());
		msg.add_action(act);
		msg.data(pi.data(), pi.data_len());
		byte[] buff = msg.pack();
		conn.send(buff, msg.length());
	}

	@Override
	public void message_callback(OFConnection conn, short type, byte[] data,
			long len) {
		if (type == 10) {
			Integer conn_id = conn.get_id();
			PacketIn pi = new PacketIn();
			pi.unpack(data);
			Integer in_port = pi.in_port();
			Long dl_src = get_long_mac(Arrays.copyOfRange(pi.data(), 6, 12));
			Long dl_dst = get_long_mac(Arrays.copyOfRange(pi.data(), 0, 6));
			if (dl_dst == 0xffffffffff0000L) {
				flood(conn, pi);
			} else {
				Map<Long, Integer> mac_to_port = (Map<Long, Integer>) table
						.get(conn_id);
				if (mac_to_port == null) {
					table.put(conn_id,
							mac_to_port = new HashMap<Long, Integer>());
				}
				mac_to_port.put(dl_src, in_port);

				// Flood
				if (!mac_to_port.containsKey(dl_dst)) {
					flood(conn, pi);
				}
				// Install FlowMod
				else {
					Integer port = mac_to_port.get(dl_dst);
					Match m = new Match();
					m.dl_src(new EthAddress(ByteBuffer.allocate(8)
							.putLong(dl_src).array()));
					m.dl_dst(new EthAddress(ByteBuffer.allocate(8)
							.putLong(dl_dst).array()));
					FlowMod msg = new FlowMod();
					msg.cookie(new BigInteger("123"));
					msg.command((short) 0);
					msg.priority(5);
					msg.idle_timeout(60);
					msg.hard_timeout(300);
					OutputAction act = new OutputAction(port.intValue(), 1024);
					msg.add_action(act);
					msg.match(m);
					msg.buffer_id(pi.buffer_id());
					byte[] buff = msg.pack();
					conn.send(buff, msg.length());
				}
			}
		}
	}

	public LearningSwitch(String address, int port, int nthreads) {
		super(address, port, nthreads);
	}

	public static void main(String argv[]) {
		LearningSwitch l2 = new LearningSwitch("0.0.0.0", 6653, 1);
		l2.start(true);
	}
}
