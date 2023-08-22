#/**
#* \addtogroup multicast
# * @{
# */
#/**
# * \file
# *					This file implements multicast
# *
# * \author
# *					Guru Mehar Rachaputi
# *					Anders Isberg
# *	
# * \reviewer
# * 				Anders Isberg
# *
# */

CONTIKI_PROJECT = root intermediate sink
all: $(CONTIKI_PROJECT)

# nrf52dk only supports slave mode, i.e., with no routing
PLATFORMS_EXCLUDE = nrf52dk
# does not fit on sky and z1 motes
PLATFORMS_EXCLUDE += sky z1

CONTIKI = /home/sony/thesis/os/contiki-ng

# Set MCAST = (ESMRF | SMRF | MPL) one of them to switch multicast engines
MCAST ?= MPL

MAKE_MAC ?= MAKE_MAC_TSCH

include $(CONTIKI)/Makefile.identify-target
MODULES_REL += $(TARGET)

include $(CONTIKI)/Makefile.dir-variables
MODULES += $(CONTIKI_NG_NET_DIR)/ipv6/multicast
MODULES += $(CONTIKI_NG_SERVICES_DIR)/shell

# force Orchestra from command line
MAKE_WITH_ORCHESTRA ?= 1

# RPL storing mode?
MAKE_WITH_STORING_ROUTING ?= 1
# Orchestra link-based rule? (Works only if Orchestra & storing mode routing is enabled)
MAKE_WITH_LINK_BASED_ORCHESTRA ?= 1
# Use the Orchestra root rule?
MAKE_WITH_ORCHESTRA_ROOT_RULE ?= 0

ifeq ($(MCAST),ESMRF)
  # Storage based routing must be used
	MAKE_WITH_STORING_ROUTING = 1
	CFLAGS += -DRPL_CONF_MOP=RPL_MOP_STORING_MULTICAST
	CFLAGS += -DUIP_MCAST6_CONF_ENGINE=UIP_MCAST6_ENGINE_ESMRF
else ifeq ($(MCAST), SMRF)
	# Storage based routing must be used
	MAKE_WITH_STORING_ROUTING = 1
	CFLAGS += -DRPL_CONF_MOP=RPL_MOP_STORING_MULTICAST
	CFLAGS += -DUIP_MCAST6_CONF_ENGINE=UIP_MCAST6_ENGINE_SMRF
else ifeq ($(MCAST), MPL)
  # Works for none and storage based routing
	CFLAGS += -DUIP_MCAST6_CONF_ENGINE=UIP_MCAST6_ENGINE_MPL
endif

include $(CONTIKI)/Makefile.dir-variables
MODULES += $(CONTIKI_NG_SERVICES_DIR)/shell

ORCHESTRA_EXTRA_RULES = &unicast_per_neighbor_rpl_ns

ifeq ($(MAKE_WITH_ORCHESTRA),1)
 MODULES += $(CONTIKI_NG_SERVICES_DIR)/orchestra

 ifeq ($(MAKE_WITH_STORING_ROUTING),1)
	 ifeq ($(MAKE_WITH_LINK_BASED_ORCHESTRA),1)
		 # enable the `link_based` rule
		 ORCHESTRA_EXTRA_RULES = &unicast_per_neighbor_link_based
	 else
		 # enable the `rpl_storing` rule
		 ORCHESTRA_EXTRA_RULES = &unicast_per_neighbor_rpl_storing
	 endif

	 ifeq ($(MAKE_WITH_ORCHESTRA_ROOT_RULE),1)
		 # add the root rule
		 ORCHESTRA_EXTRA_RULES +=,&special_for_root
	 endif
 else
	 ifeq ($(MAKE_WITH_LINK_BASED_ORCHESTRA),1)
		 $(error "Inconsistent configuration: link-based Orchestra requires routing info")
	 endif

	 ifeq ($(MAKE_WITH_ORCHESTRA_ROOT_RULE),1)
		$(error "Inconsistent configuration: NS rule and root rule conflicts!")
	 endif
 endif

 # pass the Orchestra rules to the compiler
 CFLAGS += -DORCHESTRA_CONF_RULES="{&eb_per_time_source,$(ORCHESTRA_EXTRA_RULES),&default_common}"
endif

ifeq ($(MAKE_WITH_STORING_ROUTING),1)
 MAKE_ROUTING = MAKE_ROUTING_RPL_CLASSIC
endif

include $(CONTIKI)/Makefile.include
