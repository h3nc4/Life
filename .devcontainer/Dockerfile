FROM debian:bookworm

# Install and set up locales for C, install C development tools and other useful packages
RUN apt-get update && \
	DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends locales && \
	echo "en_US.UTF-8 UTF-8" >/etc/locale.gen && \
	locale-gen en_US.UTF-8 && \
	update-locale LANG=en_US.UTF-8 LC_ALL=en_US.UTF-8 && \
	\
	DEBIAN_FRONTEND=noninteractive apt-get upgrade -y --no-install-recommends \
	bash-completion \
	ca-certificates \
	doas \
	gcc \
	gdb \
	git \
	gnupg \
	less \
	libc-dev \
	libx11-dev \
	libxext-dev \
	libxinerama-dev \
	make \
	man-db \
	nano \
	novnc \
	procps \
	ssh-client \
	wget \
	x11vnc \
	xvfb \
	&& \
	apt-get clean && \
	rm -rf /var/lib/apt/lists/*

# Create a non-root user and give him doas permissions
RUN groupadd dev && \
	useradd -m -g dev -u 1000 -s /bin/bash dev && \
	echo "permit nopass nolog keepenv dev as root" > /etc/doas.conf && \
	echo "[ -f \"\$HOME/.config/bash/bashrc\" ] && . \"\$HOME/.config/bash/bashrc\"" >>/etc/bash.bashrc

# Xvfb
ENV DISPLAY=:99
RUN mkdir -p /tmp/.X11-unix && \
	chmod 1777 /tmp/.X11-unix

# Set up init script
COPY --chown=1000:1000 entrypoint /entrypoint
RUN chmod +x /entrypoint

# Run as non-root user
USER dev
