package com.liyihuanx.myndk;

/**
 * @author liyihuan
 * @date 2022/03/25
 * @Description
 */
public class UserBean {

	private String username;
	private int id;

	public String getUsername() {
		return username == null ? "" : username;
	}

	public void setUsername(String username) {
		this.username = username;
	}

	public int getId() {
		return id;
	}

	public void setId(int id) {
		this.id = id;
	}

	@Override
	public String toString() {
		return "UserBean{" +
				"username='" + username + '\'' +
				", id=" + id +
				'}';
	}
}
